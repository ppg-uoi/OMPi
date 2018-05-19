/*
  OMPi OpenMP Compiler
  == Copyright since 2001 the OMPi Team
  == Dept. of Computer Science & Engineering, University of Ioannina

  This file is part of OMPi.

  OMPi is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  OMPi is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OMPi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <fcntl.h>
#include <err.h>
#include <elf.h>
#include <unistd.h>

#include "loader.h"

#define EMEM_SIZE (0x02000000)
#define MAX_NUM_WRITE_PACKETS 256
#define MAX_BUFFER_TO_SERVER_SIZE (MAX_NUM_WRITE_PACKETS * 8)

#define BUFSIZE 1000  // should be sufficient for an SREC record
#define BIGBUFSIZE 4096000

#define diag(vN)   if (my_e_load_verbose >= vN)

e_loader_diag_t my_e_load_verbose = 0;
kernel_code_t kernel_buffer[MAX_KERNEL_FILES]; /* TODO: Upper limit in different kernels... */

/* TODO: The following two functions are specific to old esdk, must alter E_REG_CORE_RESET */
ssize_t my_ee_write_reg(e_epiphany_t *dev, unsigned row, unsigned col,
                        off_t to_addr, int data)
{
	int *pto;
	ssize_t size;

	if (to_addr >= E_CORE_GP_REG_BASE)
		to_addr = to_addr - E_CORE_GP_REG_BASE;

	size = sizeof(int);

	if (((to_addr + size) > dev->core[row][col].regs.map_size) || (to_addr < 0))
	{
		diag(H_D2) { fprintf(stderr, "ee_write_reg(): writing to to_addr=0x%08x, size=%d, map_size=0x%x\n", (uint) to_addr, (uint) size, (uint) dev->core[row][col].regs.map_size); }
		warnx("ee_write_reg(): Buffer range is out of bounds.");
		return E_ERR;
	}

	pto = (int *)(dev->core[row][col].regs.base + to_addr);

	diag(H_D2) { fprintf(stderr, "ee_write_reg(): writing to to_addr=0x%08x, pto=0x%08x\n", (uint) to_addr, (uint) pto); }

	*pto = data;

	return sizeof(int);
}

int my_e_reset_group(e_epiphany_t *dev[], int num_of_groups)
{
	int RESET0 = 0x0;
	int RESET1 = 0x1;
	int CONFIG = 0x01000000;
	int row, col;
	int i;

	for (i = 0; i < num_of_groups; i++)
	{
		diag(H_D1) { fprintf(stderr, "e_reset_group(): halting core...\n"); }

		for (row = 0; row < dev[i]->rows; row++)
			for (col = 0; col < dev[i]->cols; col++)
				e_halt(dev[i], row, col);

		diag(H_D1) { fprintf(stderr, "e_reset_group(): pausing DMAs.\n"); }

		for (row = 0; row < dev[i]->rows; row++)
			for (col = 0; col < dev[i]->cols; col++)
				e_write(dev[i], row, col, E_REG_CONFIG, &CONFIG, sizeof(unsigned));
	}

	usleep(100000);

	for (i = 0; i < num_of_groups; i++)
	{
		diag(H_D1) { fprintf(stderr, "e_reset_group(): resetting cores...\n"); }

		for (row = 0; row < dev[i]->rows; row++)
			for (col = 0; col < dev[i]->cols; col++)
			{
				my_ee_write_reg(dev[i], row, col, E_REG_CORE_RESET, RESET1);
				my_ee_write_reg(dev[i], row, col, E_REG_CORE_RESET, RESET0);
			}

		diag(H_D1) { fprintf(stderr, "e_reset_group(): done.\n"); }
	}

	return E_OK;
}

e_return_stat_t my_ee_process_SREC(char *executable, e_epiphany_t *pEpiphany,
                                   e_mem_t *pEMEM, int row, int col, int is_new_kernel, int buf_index)
{
	typedef enum {S0, S3, S7} SrecSel;
	FILE      *srecStream;
	int      i;
	char     buf[BUFSIZE], *pbuf;
	e_bool_t insection;
	e_bool_t islocal, isonchip;
	unsigned CoreID;
	int      status = E_OK;
	char    *file_buffer;

	insection = E_FALSE;
	islocal   = E_FALSE;
	isonchip  = E_FALSE;

	file_buffer = kernel_buffer[buf_index].buf;

	diag(L_D1) { fprintf(stderr, "ee_process_SREC(): loading core (%d,%d).\n", row, col); }

	if (is_new_kernel)
	{
		srecStream = fopen(executable, "r");
		if (srecStream == NULL)
		{
			fprintf(stderr, "Error: Can't open SREC file: %s\n", executable);
			return E_ERR;
		}

		rewind(srecStream);
		unsigned lineN = 0;
		char *dummy    = NULL;

		while (!feof(srecStream) && !ferror(srecStream))
		{
			dummy = fgets(buf, BUFSIZE, srecStream);
			/* copy line to our buffer for future usage */
			strcpy(&(file_buffer[lineN * BUFSIZE]), buf);

			if (!feof(srecStream) && !ferror(srecStream))
			{
				diag(L_D3) { fprintf(stderr, "ee_process_SREC(): %s", buf); }

				// RECORD TYPE
				SrecSel sSel;
				unsigned addrSize;

				if (buf[0] != 'S')
				{
					warnx("Error: Invalid record format in SREC file line ");
					warnx("%d: \"%s\"\n", lineN, buf);
					return E_ERR;
				}

				if (buf[1] == '0')
				{
					sSel = S0;
					addrSize = 4;
					if (insection == E_TRUE)
					{
						warnx("Error: S0 record found inside a section in SREC file line ");
						warnx("%d: \"%s\"\n", lineN, buf);
						status = E_WARN;
						continue; // TODO: bail out with error code
					}
					else
						insection = E_TRUE;
				}
				else
					if (buf[1] == '3')
					{
						sSel = S3;
						addrSize = 8;
						if (insection == E_FALSE)
						{
							warnx("Error: S3 record found outside of a section in SREC file line ");
							warnx("%d: \"%s\"\n", lineN, buf);
							status = E_WARN;
							continue; // TODO: bail out with error code
						}
					}
					else
						if (buf[1] == '7')
						{
							sSel = S7;
							addrSize = 8;
							if (insection == E_FALSE)
							{
								warnx("Error: S7 record found outside of a section in SREC file line ");
								warnx("%d: \"%s\"\n", lineN, buf);
								status = E_WARN;
								continue; // TODO: bail out with error code
							}
							else
								insection = E_FALSE;
						}
						else
						{
							warnx("Error: Invalid record types (valid types are S0, S3 and S7) in SREC file line ");
							warnx("%d: \"%s\"\n", lineN, buf);
							status = E_WARN;
							continue; // TODO: bail out with error code
						}


				// BYTES COUNT
				char byteCountStr[5];
				byteCountStr[0] = '0';
				byteCountStr[1] = 'x';
				byteCountStr[2] = buf[2];
				byteCountStr[3] = buf[3];
				byteCountStr[4] = '\0';
				unsigned byteCount = strtol(byteCountStr, NULL, 16);
				if (byteCount > 0)
				{
					byteCount = byteCount - (addrSize / 2) /* addr */ - 1 /* checksum */;
					diag(L_D3) { fprintf(stderr, "ee_process_SREC(): record length = %d\n", byteCount); }
				}
				else
				{
					warnx("Error: Wrong record format in SREC file line ");
					warnx("%d: \"%s\"\n", lineN, buf);
					status = E_WARN;
					continue;
				}


				// ADDRESS
				unsigned long addrHex;

				char addrBuf[9];
				strncpy(addrBuf, buf + 4, addrSize);
				addrBuf[addrSize] = '\0';
				addrHex = strtoll(addrBuf, NULL, 16);
				if (addrHex & 0xfff00000)
				{
					// This is a global address. Check if address is on an eCore.
					islocal  = E_FALSE;
					isonchip = e_is_addr_on_chip((void *) addrHex);
				}
				else
				{
					// This is a local address.
					islocal  = E_TRUE;
					isonchip = E_TRUE;
				}
				diag(L_D3) { fprintf(stderr, "ee_process_SREC(): address: 0x%08x\n", (uint) addrHex); }


				// DATA
				if (sSel == S0)
				{
					// Start of Core section
					char dataBuf[5];

					pbuf = buf + 4;
					strncpy(dataBuf, pbuf, 4);
					dataBuf[4] = '\0';

					diag(L_D3) { fprintf(stderr, "ee_process_SREC(): %x\n", (uint) dataBuf); }
				}


				if (sSel == S3)
				{
					// Core data record
					unsigned char Data2Send[MAX_BUFFER_TO_SERVER_SIZE];
					unsigned int globrow, globcol;

					assert(byteCount <= MAX_BUFFER_TO_SERVER_SIZE);
					diag(L_D3) { fprintf(stderr, "ee_process_SREC(): copying the data (%d bytes)", byteCount); }

					// convert text to bytes
					pbuf = buf + 4 + addrSize;
					for (i = 0; i < byteCount; i++)
					{
						char dataBuf[3];
						dataBuf[0] = *(pbuf++);
						dataBuf[1] = *(pbuf++);
						dataBuf[2] = '\0';

						unsigned long dataHex = strtol(dataBuf, NULL, 16);
						Data2Send[i] = dataHex;

						diag(L_D4) { fprintf(stderr, "ee_process_SREC():  %s", dataBuf); }
					}

					if (islocal)
					{
						// If this is a local address
						diag(L_D3) { fprintf(stderr, " to core (%d,%d)\n", row, col); }
						ee_write_buf(pEpiphany, row, col, addrHex, Data2Send, byteCount);
					}
					else
						if (isonchip)
						{
							// If global address, check if address is of an eCore.
							CoreID = addrHex >> 20;
							ee_get_coords_from_id(pEpiphany, CoreID, &globrow, &globcol);
							diag(L_D3) { fprintf(stderr, " to core (%d,%d)\n", globrow, globcol); }
							ee_write_buf(pEpiphany, globrow, globcol, addrHex & ~(0xfff00000), Data2Send,
							             byteCount);
						}
						else
						{
							// If it is not on an eCore, it is on external memory.
							diag(L_D3) { fprintf(stderr, " to external memory.\n"); }
							if ((addrHex >= pEMEM->ephy_base)
							    && (addrHex < (pEMEM->ephy_base + pEMEM->emap_size)))
							{
								diag(L_D3) { fprintf(stderr, "ee_process_SREC(): converting virtual (0x%08x) ", (uint) addrHex); }
								addrHex = addrHex - (pEMEM->ephy_base - pEMEM->phy_base);
								diag(L_D3) { fprintf(stderr, "to physical (0x%08x)...\n", (uint) addrHex); }
							}
							diag(L_D3) { fprintf(stderr, "ee_process_SREC(): converting physical (0x%08x) ", (uint) addrHex); }
							addrHex = addrHex - pEMEM->phy_base;
							diag(L_D3) { fprintf(stderr, "to offset (0x%08x)...\n", (uint) addrHex); }
							ee_mwrite_buf(pEMEM, addrHex, Data2Send, byteCount);
						}
				}


				if (sSel == S7)
				{
					// End of Core section
					char startAddrofProg[9];

					pbuf = buf + 4;
					strncpy(startAddrofProg, pbuf, addrSize);
					startAddrofProg[addrSize] = '\0';
					unsigned long startOfProrgram = strtol(startAddrofProg, NULL, 16);
					if (startOfProrgram != 0)
					{
						warnx("Warning: non zero _start address. The start of program is detected in the address ");
						warnx("%x\n", (unsigned int) startOfProrgram);
						status = E_WARN;
					}
				}
				lineN++;
			}
		}

		kernel_buffer[buf_index].lines = lineN;

		fclose(srecStream);
	}
	else /* I have loaded the same file in the past... */
	{
		unsigned lineN = 0;
		char *c_buf;

		while (lineN < kernel_buffer[buf_index].lines)
		{
			c_buf = &(file_buffer[lineN * BUFSIZE]);
			diag(L_D3) { fprintf(stderr, "ee_process_SREC(): %s", c_buf); }

			// RECORD TYPE
			SrecSel sSel;
			unsigned addrSize;

			if (c_buf[0] != 'S')
			{
				warnx("Error: Invalid record format in SREC file line ");
				warnx("%d: \"%s\"\n", lineN, c_buf);
				return E_ERR;
			}

			if (c_buf[1] == '0')
			{
				sSel = S0;
				addrSize = 4;
				insection = E_TRUE;

			}
			else
				if (c_buf[1] == '3')
				{
					sSel = S3;
					addrSize = 8;
				}
				else
					if (c_buf[1] == '7')
					{
						sSel = S7;
						addrSize = 8;
						insection = E_FALSE;
					}


			// BYTES COUNT
			char byteCountStr[5];
			byteCountStr[0] = '0';
			byteCountStr[1] = 'x';
			byteCountStr[2] = c_buf[2];
			byteCountStr[3] = c_buf[3];
			byteCountStr[4] = '\0';
			unsigned byteCount = strtol(byteCountStr, NULL, 16);

			byteCount = byteCount - (addrSize / 2) /* addr */ - 1 /* checksum */;
			diag(L_D3) { fprintf(stderr, "ee_process_SREC(): record length = %d\n", byteCount); }

			// ADDRESS
			unsigned long addrHex;

			char addrBuf[9];
			strncpy(addrBuf, c_buf + 4, addrSize);
			addrBuf[addrSize] = '\0';
			addrHex = strtoll(addrBuf, NULL, 16);
			if (addrHex & 0xfff00000)
			{
				// This is a global address. Check if address is on an eCore.
				islocal  = E_FALSE;
				isonchip = e_is_addr_on_chip((void *) addrHex);
			}
			else
			{
				// This is a local address.
				islocal  = E_TRUE;
				isonchip = E_TRUE;
			}
			diag(L_D3) { fprintf(stderr, "ee_process_SREC(): address: 0x%08x\n", (uint) addrHex); }


			// DATA
			if (sSel == S0)
			{
				// Start of Core section
				char dataBuf[5];

				pbuf = c_buf + 4;
				strncpy(dataBuf, pbuf, 4);
				dataBuf[4] = '\0';

				diag(L_D3) { fprintf(stderr, "ee_process_SREC(): %x\n", (uint) dataBuf); }
			}


			if (sSel == S3)
			{
				// Core data record
				unsigned char Data2Send[MAX_BUFFER_TO_SERVER_SIZE];
				unsigned int globrow, globcol;

				assert(byteCount <= MAX_BUFFER_TO_SERVER_SIZE);
				diag(L_D3) { fprintf(stderr, "ee_process_SREC(): copying the data (%d bytes)", byteCount); }

				// convert text to bytes
				pbuf = c_buf + 4 + addrSize;
				for (i = 0; i < byteCount; i++)
				{
					char dataBuf[3];
					dataBuf[0] = *(pbuf++);
					dataBuf[1] = *(pbuf++);
					dataBuf[2] = '\0';

					unsigned long dataHex = strtol(dataBuf, NULL, 16);
					Data2Send[i] = dataHex;

					diag(L_D4) { fprintf(stderr, "ee_process_SREC():  %s", dataBuf); }
				}

				if (islocal)
				{
					// If this is a local address
					diag(L_D3) { fprintf(stderr, " to core (%d,%d)\n", row, col); }
					ee_write_buf(pEpiphany, row, col, addrHex, Data2Send, byteCount);
				}
				else
					if (isonchip)
					{
						// If global address, check if address is of an eCore.
						CoreID = addrHex >> 20;
						ee_get_coords_from_id(pEpiphany, CoreID, &globrow, &globcol);
						diag(L_D3) { fprintf(stderr, " to core (%d,%d)\n", globrow, globcol); }
						ee_write_buf(pEpiphany, globrow, globcol, addrHex & ~(0xfff00000), Data2Send,
						             byteCount);
					}
					else
					{
						// If it is not on an eCore, it is on external memory.
						diag(L_D3) { fprintf(stderr, " to external memory.\n"); }
						if ((addrHex >= pEMEM->ephy_base)
						    && (addrHex < (pEMEM->ephy_base + pEMEM->emap_size)))
						{
							diag(L_D3) { fprintf(stderr, "ee_process_SREC(): converting virtual (0x%08x) ", (uint) addrHex); }
							addrHex = addrHex - (pEMEM->ephy_base - pEMEM->phy_base);
							diag(L_D3) { fprintf(stderr, "to physical (0x%08x)...\n", (uint) addrHex); }
						}
						diag(L_D3) { fprintf(stderr, "ee_process_SREC(): converting physical (0x%08x) ", (uint) addrHex); }
						addrHex = addrHex - pEMEM->phy_base;
						diag(L_D3) { fprintf(stderr, "to offset (0x%08x)...\n", (uint) addrHex); }
						ee_mwrite_buf(pEMEM, addrHex, Data2Send, byteCount);
					}
			}


			if (sSel == S7)
			{
				// End of Core section
				char startAddrofProg[9];

				pbuf = c_buf + 4;
				strncpy(startAddrofProg, pbuf, addrSize);
				startAddrofProg[addrSize] = '\0';
				unsigned long startOfProrgram = strtol(startAddrofProg, NULL, 16);
				if (startOfProrgram != 0)
				{
					warnx("Warning: non zero _start address. The start of program is detected in the address ");
					warnx("%x\n", (unsigned int) startOfProrgram);
					status = E_WARN;
				}
			}
			lineN++;
		}
	}

	return status;
}

int my_e_load_group(char *executable, e_epiphany_t *dev, unsigned row,
                    unsigned col, unsigned rows, unsigned cols, e_bool_t start)
{
	e_mem_t      emem, *pemem;
	unsigned int irow, icol;
	int          status;
	FILE        *fp;
	char         hdr[5] = {'\0', '\0', '\0', '\0', '\0'};
	char         elfHdr[4] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};
	char         srecHdr[2] = {'S', '0'};
	e_bool_t     iself;
	e_return_stat_t retval;
	int          buf_index;
	int          is_new_kernel = 1;

	status = E_OK;
	iself  = E_FALSE;

	pemem = &emem;

	if (dev && pemem)
	{
		// Allocate External DRAM for the epiphany executable code
		// TODO: this is barely scalable. Really need to test ext. mem size to load
		// and possibly split the ext. mem accesses into 1MB chunks.

		if (e_alloc(pemem, 0, EMEM_SIZE))
		{
			warnx("\nERROR: Can't allocate external memory buffer!\n\n");
			return E_ERR;
		}

		if (executable[0] != '\0')
		{
			/* Search for a previous kernel buffer */
			/* No lock needed, only a HOST thread at a time can offload code */
			for (buf_index = 0; buf_index < PARALLELLA_CORES; buf_index++)
			{
				if (kernel_buffer[buf_index].executable != NULL)
				{
					if (strcmp(kernel_buffer[buf_index].executable, executable) == 0)
					{
						is_new_kernel = 0;
						break;
					}
				}
				else
				{
					kernel_buffer[buf_index].executable = (char *)malloc((strlen(
					                                                        executable) + 1) * sizeof(char));
					kernel_buffer[buf_index].buf        = (char *)malloc(BIGBUFSIZE * sizeof(char));
					strcpy(kernel_buffer[buf_index].executable, executable);
					break;
				}
			}

			/**************************************************************************/
			int hdr_index;
			if (is_new_kernel)
			{
				if ((fp = fopen(executable, "rb")) != NULL)
				{
					fseek(fp, 0, SEEK_SET);
					if(fread(hdr, 1, 4, fp) <= 0)
					{
						warnx("ERROR: Can't read file \"%s\".\n", executable);
						return E_ERR;
					}
					
					fclose(fp);

					for (hdr_index = 0; hdr_index < 5; hdr_index++)
						kernel_buffer[buf_index].hdr[hdr_index] = hdr[hdr_index];
				}
				else
				{
					warnx("ERROR: Can't open executable file \"%s\".\n", executable);
					e_free(pemem);
					return E_ERR;
				}
			}
			else
			{
				for (hdr_index = 0; hdr_index < 5; hdr_index++)
					hdr[hdr_index] = kernel_buffer[buf_index].hdr[hdr_index];
			}

			/**************************************************************************/

			if (!strncmp(hdr, elfHdr, 4))
			{
				iself = E_TRUE;
				diag(L_D1) { fprintf(stderr, "e_load_group(): loading ELF file %s ...\n", executable); }
			}
			else
				if (!strncmp(hdr, srecHdr, 2))
				{
					diag(L_D1) { fprintf(stderr, "e_load_group(): loading SREC file %s ...\n", executable); }
				}
				else
				{
					diag(L_D1)
					{
						fprintf(stderr, "e_load_group(): Executable header %02x %02x %02x %02x\n",
						        hdr[0], hdr[1], hdr[2], hdr[3]);
					}
					warnx("ERROR: Can't load executable file: unidentified format.\n");
					e_free(pemem);
					return E_ERR;
				}

			for (irow = row; irow < (row + rows); irow++)
				for (icol = col; icol < (col + cols); icol++)
				{
					if (iself)
						printf("Error! This is an elf file!\n");
					else
						retval = my_ee_process_SREC(executable, dev, pemem, irow, icol, is_new_kernel,
						                            buf_index);
					//retval = ee_process_SREC(executable, dev, pemem, irow, icol);

					if (retval == E_ERR)
					{
						warnx("ERROR: Can't load executable file \"%s\".\n", executable);
						e_free(pemem);
						return E_ERR;
					}
					else
						ee_set_core_config(dev, pemem, irow, icol);
				}

			if (start)
				for (irow = row; irow < (row + rows); irow++)
					for (icol = col; icol < (col + cols); icol++)
					{
						diag(L_D1) { fprintf(stderr, "e_load_group(): send SYNC signal to core (%d,%d)...\n", irow, icol); }
						e_start(dev, irow, icol);
						diag(L_D1) { fprintf(stderr, "e_load_group(): done.\n"); }
					}

			diag(L_D1) { fprintf(stderr, "e_load_group(): done loading.\n"); }
		}

		e_free(pemem);
		diag(L_D1) { fprintf(stderr, "e_load_group(): closed connection.\n"); }
	}
	else
	{
		warnx("ERROR: Can't connect to Epiphany or external memory.\n");
		return E_ERR;
	}

	diag(L_D1) { fprintf(stderr, "e_load_group(): leaving loader.\n"); }

	return status;
}

void init_kernel_buffers(void)
{
	int i;

	for (i = 0; i < PARALLELLA_CORES; i++)
		kernel_buffer[i].executable = NULL;
}

