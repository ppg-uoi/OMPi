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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* x_map.c -- mapping-related code */

#include "ast_copy.h"
#include "x_map.h"
#include "x_clauses.h"
#include "x_decltarg.h"
#include "symtab.h"
#include "ompi.h"
#include "x_target.h"

//#define MAP_DBG

#define FuncnameCall(name,params) FuncCallStmt(IdentName(name),params)


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     MAPPING-UNMAPPING-UPDATING CALLS                          *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/** 
 * Given a variable e, it fetches essential info needed for its mapping
 * (address of first array section element, array section length etc).
 * @param e        The variable (symbol)
 * @param onlyexisting False nowadays; was true when we wanted to produce calls
 *                 only for variables that were already in the data environment.                 
 * @param itemaddr (ret) Expression with pointer to the beggining of var
 * @param nbytes   (ret) Expression with the number of bytes to be mapped
 * @param addrlb   (ret) Expression with pointer to the 1st byte to be mapped
 * @param isptr    (ret) Expression returning true if e is pointer
 * @param always   (ret) true if in a map clause with the "always" modifier
 * @return         True unless ingored (onlexisting && !isindevenv)
 */
static 
int map_var_get_info(setelem(vars) e, bool onlyexisting,
                     astexpr *itemaddr, astexpr *nbytes, astexpr *addrlb,
                     astexpr *isptr, int *always)
{
	int     ptrarrsec;
	stentry sym;
	ompxli  xl;

	sym = symtab_get(stab, e->key, IDNAME);

#ifdef MAP_DBG
	fprintf(stderr, "  [%s]:\n\t%8s: clause = %d, isindevenv = %d (%s)\n", 
		__FUNCTION__,
		sym->key->name, e->value.clause, sym->isindevenv,
		(onlyexisting && !sym->isindevenv) ? "ignoring" : "ok");
#endif
	
	/* If onlyexisting is true we only create calls for variables that are
	 * already in a device data environment (only for dev env vars).
	 * FIXME: This should ONLY be checked for target/target data vars, 
	 *        not for unstructured mappings. Hopefully onlyexisting is false then.
	 */
	if (onlyexisting && !sym->isindevenv)
		return 0;

	/* If found in a (map) clause, use the element's ptr to get the xlitem;
	 * otherwise, get the xlitem from the symbol table (thus if a var is an 
	 * array section, the exact same section will be used).
	 * xl will be NULL for injected globals like reduction/critical locks
	 */
	xl = (ompxli)( (e->value.clause != OCNOCLAUSE) ? e->value.ptr : sym->pval );

#ifdef MAP_DBG
	fprintf(stderr, "\t%8s: xl = %s, xl->xlitype = %s\n", 
		sym->key->name, xl ? "ok" : "null",
		xl ? (xl->xlitype == OXLI_IDENT ? "ident" : "arrsec") : "-");
#endif
	
	/* If it is not in a device data environment, treat it as an OXLI_IDENT,
	 * unless it was found on a clause (this occurs in target data).
	 * Same goes if it is on the device environment but xl is NULL.
	 */
	if ((!sym->isindevenv && e->value.clause == OCNOCLAUSE) || 
	    xl == NULL || xl->xlitype == OXLI_IDENT)
	{
		ptrarrsec = decl_ispointer(sym->decl);
		if (itemaddr)
			*itemaddr = UOAddress(Identifier(e->key));
		/* Pointers are treated as 0-length array sections */
		if (nbytes)
			*nbytes   = ptrarrsec ? numConstant(0) : Sizeof(Identifier(e->key));
		if (addrlb)
			*addrlb   = ast_expr_copy(*itemaddr);
	}
	else    /* Array section */
	{
		ptrarrsec = !(sym->isarray);
		xc_xlitem_copy_info(xl, itemaddr, nbytes, addrlb);
	}
	if (isptr)
		*isptr = numConstant(ptrarrsec);
	if (always)
		*always = (e->value.clause != OCNOCLAUSE && e->value.clmod == OCM_always);
	
	return 1;
}


/** 
 * Given a variable e, it fetches essential info needed for its unmapping
 * (well, just its address).
 * @param e        The variable (symbol)
 * @param onlyexisting ...
 * @param itemaddr (ret) Expression with pointer to the beggining of var
 * @param always   (ret) True if it is on map clause with an ALWAYS modifier
 * @param delete   (ret) True if it is on map(delete:) clause
 * @return         True unless ingored (onlexisting && !isindevenv)
 */
static 
int unmap_var_get_info(setelem(vars) e, bool onlyexisting, astexpr *itemaddr,
                       bool *always, bool *delete)
{
	stentry sym;

	sym = symtab_get(stab, e->key, IDNAME);

#ifdef MAP_DBG
	fprintf(stderr, "  [%s]:\n\t%8s: clause = %d, isindevenv = %d (%s)\n", 
		__FUNCTION__,
		sym->key->name, e->value.clause, sym->isindevenv,
		(onlyexisting && !sym->isindevenv) ? "ignoring" : "ok");
#endif
	
	/* If onlyexisting is true we only create calls for variables that are
	 * already in a device data environment (only for dev env vars).
	 * FIXME: This should ONLY be checked for target/target data vars, 
	 *        not for unstructured mappings. Hopefully onlyexisting is false then.
	 */
	if (onlyexisting && !sym->isindevenv)
		return 0;

	if (itemaddr)
		*itemaddr = UOAddress(Identifier(e->key));
	if (always)
		*always = (e->value.clause != OCNOCLAUSE && e->value.clmod == OCM_always);
	if (delete)
		*delete = (e->value.clause != OCNOCLAUSE && e->value.clause == OC_delete);
	return 1;
}


/* Produces a statement to map a #target or a #target data variable.
 */
aststmt xm_map_structured(setelem(vars) e, astexpr ignore, int how)
{
	astexpr itemaddr, nbytes, addrlb, isptrarrsec;
	int     always;
	extern const bool allvarsindevenv;   // FIXME
	
	if (!map_var_get_info(e, !allvarsindevenv, 
		                    &itemaddr, &nbytes, &addrlb, &isptrarrsec, &always))
		return NULL;

	/* Take care of forced update */
	if (how != UPDATE_DISABLE && always)
		how = UPDATE_FORCED;
	return FuncnameCall("ort_map_tdvar", Comma5(itemaddr, nbytes, addrlb, 
		                  isptrarrsec, numConstant(how)));
}


/* Produces a statement to map a #target declare link or a #target exit data 
 * variable.
 */
aststmt xm_map_unstructured(setelem(vars) e, astexpr devexpr, int how)
{
	astexpr itemaddr, nbytes, addrlb, isptrarrsec;
	int     always;
	
	if (!map_var_get_info(e, false, 
		                    &itemaddr, &nbytes, &addrlb, &isptrarrsec, &always))
		return NULL;

	/* Take care of forced update */
	if (how != UPDATE_DISABLE && always)
		how = UPDATE_FORCED;
	return FuncnameCall("ort_map_var_dev", Comma6(itemaddr, nbytes, addrlb, 
		                  isptrarrsec,ast_expr_copy(devexpr), numConstant(how)));
}


/* Produces a statement to unmap a #target or a #target data variable.
 */
aststmt xm_unmap_structured(setelem(vars) e, astexpr ignore, int how)
{
	astexpr itemaddr;
	bool    always;

	if (!unmap_var_get_info(e, false, &itemaddr, &always, NULL))
		return NULL;   /* Never occurs */
	/* Take care of forced update */
	if (how != UPDATE_DISABLE && always)
		how = UPDATE_FORCED;
	return 
		FuncnameCall("ort_unmap_tdvar", Comma2(itemaddr, numConstant(how)));
}


/* Produces a statement to unmap a #target declare link or a #target exit data 
 * variable.
 */
aststmt xm_unmap_unstructured(setelem(vars) e, astexpr devexpr, int how)
{
	astexpr itemaddr;
	bool    always;

	if (!unmap_var_get_info(e, false, &itemaddr, &always, NULL))
		return NULL;   /* Never occurs */
	if (how != UPDATE_DISABLE && always)
		how = UPDATE_FORCED;
	return 
		FuncnameCall("ort_unmap_var_dev", 
			Comma4(itemaddr,ast_expr_copy(devexpr),numConstant(how),numConstant(0)));
}


/** 
 * Iterates over a set of variables "s" and creates mapping or unmapping calls 
 * through function "func". The generated code is returned through "stmt".
 * Called from xm_usedvars/linkvars_mappings().
 * @param s         the set of variables
 * @param stmt      (ret) the statement with all unmappings
 * @param devexpr   an expression with the device id
 * @param func      the function to apply to each element
 * @param updatehow update and refer actions
 * @param comment   a comment to preceed the unmappings
 * @return          the number of handled variables.
 */
int xm_mup_set(set(vars) s, aststmt *stmt, 
               astexpr devexpr, muper_t func, int updatehow, char *comment)
{
	int           n = 0;
	aststmt       st;
	setelem(vars) e;

#ifdef MAP_DBG
	if (s->first)
		fprintf(stderr, "  %s items:\n", comment);
#endif
	for (e = s->first; e; e = e->next)
	{
		st = (*func)(e, devexpr, updatehow);
		if (st == NULL)
			continue;
		if (n == 0)   /* Comment */
			*stmt = (*stmt == NULL) ? 
			           verbit(comment) : BlockList(*stmt, verbit(comment));
		*stmt = BlockList(*stmt, st);
		n++;
	}
	return n;
}


/** 
 * Given an xlitem e, it fetches essential info needed for its mapping
 * (address of first array section element, array section length etc).
 * @param e        The xlitem
 * @param itemaddr (ret) Expression with pointer to the beggining of var
 * @param nbytes   (ret) Expression with the number of bytes to be mapped
 * @param addrlb   (ret) Expression with pointer to the 1st byte to be mapped
 * @param isptr    (ret) Expression returning true if e is pointer
 * @param always   (ret) true if in a map clause with the "always" modifier
 * @return         True unless ingored (onlexisting && !isindevenv)
 */
static
bool map_xlitem_get_info(setelem(xlitems) e,
                         astexpr *itemaddr, astexpr *nbytes, astexpr *addrlb,
                         astexpr *isptr, int *always)
{
	if (isptr)
	{
		stentry sym = symtab_get(stab, e->key, IDNAME);

		/* FIXME: we must get rid of devenvs!!! (restructure target data) */
		if (sym->isindevenv == due2TARGDATA)
			exit_error(1, "[%s] failed for %s\n", __FUNCTION__, e->key->name);
		*isptr = numConstant( decl_ispointer(sym->decl) );
	}
	if (always)
		*always = (e->value.clause != OCNOCLAUSE && e->value.clmod == OCM_always);
	xc_xlitem_copy_info(e->value.xl, itemaddr, nbytes, addrlb);
	return true;
}


/* Produces a statement to map a #target declare link or a #target enter data 
 * variable.
 */
aststmt xm_map_xlitem(setelem(xlitems) e, astexpr devexpr, int how)
{
	astexpr itemaddr, nbytes, addrlb, isptr;
	int     always;
	
	if (!map_xlitem_get_info(e, &itemaddr, &nbytes, &addrlb, &isptr, &always))
		return NULL;

	/* Take care of forced update */
	if (how != UPDATE_DISABLE && always)
		how = UPDATE_FORCED;
	return FuncnameCall("ort_map_var_dev", Comma6(itemaddr, nbytes, addrlb, 
		                  isptr,ast_expr_copy(devexpr), numConstant(how)));
}


/* Produces a statement to map a #target declare link or a #target enter data 
 * variable.
 */
aststmt xm_unmap_xlitem(setelem(xlitems) e, astexpr devexpr, int how)
{
	astexpr itemaddr;
	int     always, update = UPDATE_DISABLE, delete = 0;
	
	if (!map_xlitem_get_info(e, &itemaddr, NULL, NULL, NULL, &always))
		return NULL;
	switch (how)
	{
		case UPDATE_DISABLE:
		case UPDATE_FORCED: 
			update = how;
			break;
		case UPDATE_NORMAL:
			/* Take care of forced update */
			update = (always) ? UPDATE_FORCED : UPDATE_NORMAL;
			break;
		case REFER_NORMAL:
			break;
		case REFER_DELETE:
			delete = 1;
			break;
		default:
			exit_error(1, "[xm_unmap_xlitem]: unexpected way\n");
	}
	return 
		FuncnameCall("ort_unmap_var_dev", 
		             Comma4(itemaddr, ast_expr_copy(devexpr), 
		                    numConstant(update), numConstant(delete)));
}


/** 
 * Iterates over a set of variables "s" and creates mapping calls through
 * function "func". The generated code is returned through "stmt".
 * Called from xm_usedvars/linkvars_mappings().
 * @param s         the set of variables
 * @param stmt      (ret) the statement with all unmappings
 * @param devexpr   an expression with the device id
 * @param func      the function to apply to each element
 * @param updatehow update and refer actions
 * @param comment   a comment to preceed the unmappings
 * @return          the number of handled variables.
 */
int xm_mup_xliset(set(xlitems) s, aststmt *stmt, 
                  astexpr devexpr, muperx_t func, int updatehow, char *comment)
{
	int     n = 0;
	aststmt st;
	setelem(xlitems) e;

#ifdef MAP_DBG
	if (s->first)
		fprintf(stderr, "  %s items:\n", comment);
#endif
	for (e = s->first; e; e = e->next)
	{
		st = (*func)(e, devexpr, updatehow);
		if (st == NULL)
			continue;
		if (n == 0)   /* Comment */
			*stmt = (*stmt == NULL) ? 
			           verbit(comment) : BlockList(*stmt, verbit(comment));
		*stmt = BlockList(*stmt, st);
		n++;
	}
	return n;
}


/** 
 * Generates the code for mapping and unmapping any used declare link variables.
 * Called when transforming #target and #targetdata.
 * @param devexpr  The device expression
 * @param con      The OpenMP construct (just to obtain the map() clauses)
 * @param maps     If not NULL, it gets the mapping statements
 * @param unmaps   If not NULL, it gets the unmapping statements
 * @param implicit All implicitly used variables in the construct (note that
 *                 outline marks ALL #declare target vars as implicit, even if
 *                 they are explicitly mapped).
 */
void xm_linkvars_mappings(astexpr devexpr, ompcon con, set(vars) implicit,
                          aststmt *maps, aststmt *unmaps)
{
	static set(vars) mapped_link_vars = NULL;
	setelem(vars)    e;
	aststmt          s;
	int              upd;
	
	/* First, gather all explicitly mapped link vars */
	set_init(vars, &mapped_link_vars);
	xc_ompcon_get_vars(con, OCMAP, OC_DontCare, mapped_link_vars);
	
	/* Second, check if there were any implicitly mapped link vars and add them */
	if (implicit)
		for (e = implicit->first; e; e = e->next)
			if (decltarg_id_isknown(e->key) && decltarg_id_clause(e->key) == OCLINK &&
					!set_get(mapped_link_vars, e->key))
				set_put(mapped_link_vars, e->key)->value.clsubt = OC_tofrom;
		
	/* Finally, (un)map them */
	*maps = *unmaps = NULL;
	for (e = mapped_link_vars->first; e; e = e->next)
		if (decltarg_id_isknown(e->key) && decltarg_id_clause(e->key) == OCLINK)
		{
			upd = (e->value.clsubt == OC_alloc || e->value.clsubt == OC_from) ?
			        UPDATE_DISABLE : 
			        ((e->value.clsubt == OCM_always) ? UPDATE_FORCED : UPDATE_NORMAL);
			if ((s = xm_map_unstructured(e, devexpr, upd)) != NULL)
			{
				if (*maps == NULL)
					*maps = verbit("/* map declare-link */");
				*maps = BlockList(*maps, s);
			}
			upd = (e->value.clsubt == OC_alloc || e->value.clsubt == OC_to) ?
			        UPDATE_DISABLE : 
			        ((e->value.clsubt == OCM_always) ? UPDATE_FORCED : UPDATE_NORMAL);
			if ((s = xm_unmap_unstructured(e, devexpr, upd)) != NULL)
			{
				if (*unmaps == NULL)
					*unmaps = verbit("/* unmap declare-link */");
				*unmaps = BlockList(*unmaps, s);
			}
		};
	set_drain(mapped_link_vars);
}


/**
 * Generates code for mapping and unmapping any used declare link variables.
 * Called when transforming #target and #targetdata.
 * @param usedvars all the sets of used variables
 * @param maps     gets the mapping statements
 * @param unmaps   gets the unmapping statements
 * @return         the total number of mapped vars
 */
int xm_usedvars_mappings(set(vars) *usedvars, aststmt *maps, aststmt *unmaps)
{
	int nvars = 0;

	*maps = *unmaps = NULL;
	/* private and firstprivate: not mapped */
	/* alloc */
	nvars += xm_mup_set(usedvars[DCT_MAPALLOC], maps, NULL, 
	                 xm_map_structured, UPDATE_DISABLE, "/* map alloc */");
	xm_mup_set(usedvars[DCT_MAPALLOC], unmaps, NULL, 
	          xm_unmap_structured, UPDATE_DISABLE, "/* unmap alloc */");
	/* to */
	nvars += xm_mup_set(usedvars[DCT_MAPTO], maps, NULL, 
	                 xm_map_structured, UPDATE_NORMAL, "/* map to */");
	xm_mup_set(usedvars[DCT_MAPTO], unmaps, NULL, 
	          xm_unmap_structured, UPDATE_DISABLE, "/* unmap to */");
	/* from */
	nvars += xm_mup_set(usedvars[DCT_MAPFROM], maps, NULL, 
	                 xm_map_structured, UPDATE_DISABLE, "/* map from */");
	xm_mup_set(usedvars[DCT_MAPFROM], unmaps, NULL, 
	          xm_unmap_structured, UPDATE_NORMAL, "/* unmap from */");
	/* tofrom */
	nvars += xm_mup_set(usedvars[DCT_MAPTOFROM], maps, NULL, 
	                 xm_map_structured, UPDATE_NORMAL, "/* map tofrom */");
	xm_mup_set(usedvars[DCT_MAPTOFROM], unmaps, NULL, 
	          xm_unmap_structured, UPDATE_NORMAL, "/* unmap tofrom */");
	/* zero-length arrsec */
	nvars += xm_mup_set(usedvars[DCT_ZLAS], maps, NULL, 
	                 xm_map_structured, UPDATE_NORMAL, "/* map zlas */");
	xm_mup_set(usedvars[DCT_ZLAS], unmaps, NULL, 
	          xm_unmap_structured, UPDATE_NORMAL, "/* unmap zlas */");
	/* Variables used in a "target" that have not been explicitly defined and
	   ARE (VVD) in a device data environment */
	nvars += xm_mup_set(usedvars[DCT_DDENV], maps, NULL, 
	                 xm_map_structured, UPDATE_NORMAL, 
	                 "/* map undefined (ddenv/tofrom) */");
	xm_mup_set(usedvars[DCT_DDENV], unmaps, NULL, 
	          xm_unmap_structured, UPDATE_NORMAL, 
	          "/* unmap undefined (ddenv/tofrom) */");
	return (nvars);
}
