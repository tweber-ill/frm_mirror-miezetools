/*
 * Symbol Table
 * @author tweber
 * @date 2013
 */

#include "symbol.h"
#include "helper/log.h"
#include <set>
#include <limits>


/*std::*/boost::hash<t_real> SymbolDouble::s_hsh;
/*std::*/boost::hash<t_int> SymbolInt::s_hsh;
/*std::*/boost::hash<t_string> SymbolString::s_hsh;


SymbolMapKey::SymbolMapKey(const t_string& _str) 
	: key(SymbolString::hash(_str)), strKey(_str), tyKey(SYMBOL_STRING) 
{}

SymbolMapKey::SymbolMapKey(t_string&& _str) 
	: key(SymbolString::hash(_str)), strKey(_str), tyKey(SYMBOL_STRING) 
{}

SymbolMapKey::SymbolMapKey(const Symbol* pSym) 
	: key(pSym->hash()), 
		strKey(pSym->GetType()==SYMBOL_STRING?pSym->print():""), 
		tyKey(pSym->GetType())
{}



const int SymbolDouble::m_defprec = std::numeric_limits<t_real>::digits10;
int SymbolDouble::m_prec = m_defprec;


Symbol::~Symbol()
{
	//log_debug("Deleting ", this, ", name: ", GetName(), ", ident: ", GetIdent());
	//log_backtrace();
}


Symbol* SymbolDouble::ToType(SymbolType stype) const
{
	Symbol *pNewSym = 0;

	if(stype == SYMBOL_DOUBLE)
	{
		pNewSym = this->clone();
	}
	else if(stype == SYMBOL_INT)
	{
		SymbolInt *pNewSymI = new SymbolInt();
		pNewSymI->SetName(this->GetName());
		pNewSymI->SetVal(t_int(this->GetVal()));

		pNewSym = pNewSymI;
	}
	else if(stype == SYMBOL_STRING)
	{
		SymbolString *pNewSymS = new SymbolString();
		pNewSymS->SetName(this->GetName());
		pNewSymS->SetVal(print());

		pNewSym = pNewSymS;
	}

	return pNewSym;
}

t_string SymbolDouble::print() const
{
	t_ostringstream ostr;
	ostr.precision(m_prec);

	ostr << GetVal();
	return ostr.str();
}

Symbol* SymbolDouble::clone() const
{
	SymbolDouble *pSym = new SymbolDouble;
	*pSym = *this;
	pSym->SetRval(1);
	pSym->SetConst(0);
	return pSym;
}

void SymbolDouble::assign(Symbol *pSym)
{
	SymbolDouble *pOther = (SymbolDouble*)pSym->ToType(GetType());
	this->SetVal(pOther->GetVal());
}

bool SymbolDouble::IsLessThan(const Symbol& sym) const
{
	return GetValDouble() < sym.GetValDouble();
}

bool SymbolDouble::IsGreaterThan(const Symbol& sym) const
{
	return GetValDouble() > sym.GetValDouble();
}




Symbol* SymbolInt::ToType(SymbolType stype) const
{
	Symbol *pNewSym = 0;

	if(stype == SYMBOL_INT)
	{
		pNewSym = this->clone();
	}
	else if(stype == SYMBOL_DOUBLE)
	{
		SymbolDouble *pNewSymD = new SymbolDouble();
		pNewSymD->SetName(this->GetName());
		pNewSymD->SetVal(t_real(this->GetVal()));

		pNewSym = pNewSymD;
	}
	else if(stype == SYMBOL_STRING)
	{
		SymbolString *pNewSymS = new SymbolString();
		pNewSymS->SetName(this->GetName());
		pNewSymS->SetVal(print());

		pNewSym = pNewSymS;
	}

	return pNewSym;
}

t_string SymbolInt::print() const
{
	t_ostringstream ostr;
	ostr << GetVal();
	return ostr.str();
}

Symbol* SymbolInt::clone() const
{
	SymbolInt *pSym = new SymbolInt;
	*pSym = *this;
	pSym->SetRval(1);
	pSym->SetConst(0);
	return pSym;
}

void SymbolInt::assign(Symbol *pSym)
{
	SymbolInt *pOther = (SymbolInt*)pSym->ToType(GetType());
	this->SetVal(pOther->GetVal());
}

bool SymbolInt::IsLessThan(const Symbol& sym) const
{
	if(sym.GetType() == SYMBOL_DOUBLE)
       		return GetValDouble() < sym.GetValDouble();

	return GetValInt() < sym.GetValInt();
}

bool SymbolInt::IsGreaterThan(const Symbol& sym) const
{
	if(sym.GetType() == SYMBOL_DOUBLE)
       		return GetValDouble() > sym.GetValDouble();

	return GetValInt() > sym.GetValInt();
}




Symbol* SymbolString::ToType(SymbolType stype) const
{
	Symbol *pNewSym = 0;

	if(stype == SYMBOL_STRING)
	{
		pNewSym = this->clone();
	}
	else if(stype == SYMBOL_INT)
	{
		t_istringstream istr(GetVal());

		SymbolInt *pNewSymI = new SymbolInt();
		pNewSymI->SetName(this->GetName());

		t_int iVal;
		istr >> iVal;
		pNewSymI->SetVal(iVal);

		pNewSym = pNewSymI;
	}
	else if(stype == SYMBOL_DOUBLE)
	{
		t_istringstream istr(GetVal());

		SymbolDouble *pNewSymD = new SymbolDouble();
		pNewSymD->SetName(this->GetName());
		t_real dVal;
		istr >> dVal;
		pNewSymD->SetVal(dVal);

		pNewSym = pNewSymD;
	}

	return pNewSym;
}

t_string SymbolString::print() const
{
	t_ostringstream ostr;
	ostr << GetVal();
	return ostr.str();
}

Symbol* SymbolString::clone() const
{
	SymbolString *pSym = new SymbolString;
	*pSym = *this;
	pSym->SetRval(1);
	pSym->SetConst(0);
	return pSym;
}

void SymbolString::assign(Symbol *pSym)
{
	SymbolString *pOther = (SymbolString*)pSym->ToType(GetType());
	this->SetVal(pOther->GetVal());
}

bool SymbolString::IsLessThan(const Symbol& sym) const
{
       	return GetVal() < sym.print();
}

bool SymbolString::IsGreaterThan(const Symbol& sym) const
{
       	return GetVal() > sym.print();
}




SymbolArray::SymbolArray(const std::initializer_list<Symbol*>& lst)
{
	m_arr.reserve(lst.size());
	for(std::initializer_list<Symbol*>::const_iterator iter=lst.begin(); iter!=lst.end(); ++iter)
		m_arr.push_back(*iter);

	UpdateIndices();
}

SymbolArray::~SymbolArray()
{
	if(!m_bDontDel)
		for(Symbol *pSym : m_arr)
			if(pSym) delete pSym;

	m_arr.clear();
}

Symbol* SymbolArray::ToType(SymbolType stype) const
{
	Symbol *pNewSym = 0;

	if(stype == SYMBOL_ARRAY)
	{
		pNewSym = this->clone();
	}
	else if(stype == SYMBOL_STRING)
	{
		SymbolString *pNewSymS = new SymbolString();
		pNewSymS->SetName(this->GetName());
		pNewSymS->SetVal(print());

		pNewSym = pNewSymS;
	}
	else
		log_err("Cannot convert array to type ", stype, ".");

	return pNewSym;
}

t_string SymbolArray::print() const
{
	t_ostringstream ostr;

	ostr << "[";
	for(unsigned int i=0; i<m_arr.size(); ++i)
	{
		const Symbol* pSym = m_arr[i];
		ostr << pSym->print();

		if(i<m_arr.size()-1)
			ostr << ", ";
	}
	ostr << "]";

	return ostr.str();
}

Symbol* SymbolArray::clone() const
{
	SymbolArray *pSym = new SymbolArray;

	pSym->GetArr().reserve(m_arr.size());

	for(Symbol *pArrSym : m_arr)
		pSym->GetArr().push_back(pArrSym->clone());

	pSym->UpdateIndices();
	return pSym;
}

void SymbolArray::assign(Symbol *pSym)
{
	SymbolArray *pOther = (SymbolArray*)pSym->ToType(GetType());
	this->m_arr = pOther->GetArr();
}

void SymbolArray::UpdateIndex(unsigned int iIdx)
{
	if(!m_arr[iIdx]) return;
	m_arr[iIdx]->SetArrPtr(this);
	m_arr[iIdx]->SetArrIdx(iIdx);
}

void SymbolArray::UpdateLastNIndices(unsigned int N)
{
	unsigned int iLast = m_arr.size();
	if(iLast == 0) return;
	iLast -= 1;

	t_arr::reverse_iterator iter = m_arr.rbegin();
	for(unsigned int i=0; i<N && iter!=m_arr.rend(); ++i, ++iter)
	{
		if(!*iter) continue;

		(*iter)->SetArrPtr(this);
		(*iter)->SetArrIdx(iLast-i);
	}
}

void SymbolArray::UpdateIndices()
{
	for(unsigned int iIdx=0; iIdx<m_arr.size(); ++iIdx)
		UpdateIndex(iIdx);
}

std::vector<t_real> SymbolArray::ToDoubleArray() const
{
	std::vector<t_real> vec;
	vec.reserve(m_arr.size());

	for(const Symbol *pSym : m_arr)
	{
		t_real dVal = ((SymbolDouble*)pSym->ToType(SYMBOL_DOUBLE))->GetVal();
		vec.push_back(dVal);
	}

	return vec;
}

void SymbolArray::FromDoubleArray(const std::vector<t_real>& vec)
{
	m_arr.reserve(m_arr.size() + vec.size());

	for(t_real d : vec)
	{
		SymbolDouble *pSym = new SymbolDouble;
		pSym->SetVal(d);
		m_arr.push_back(pSym);
	}

	UpdateIndices();
}

std::size_t SymbolArray::hash() const
{
	static const std::size_t iArrSeed = SYMBOL_ARRAY;	// unique seed

	std::size_t iSeed = iArrSeed;
	for(const Symbol* pSym : m_arr)
	{
		std::size_t iHsh = pSym->hash();
		boost::hash_combine(iSeed, iHsh);
	}
	return iSeed;
}


SymbolMap::~SymbolMap()
{
	for(t_map::value_type& val : m_map)
		if(val.second) delete val.second;

	m_map.clear();
}

Symbol* SymbolMap::ToType(SymbolType stype) const
{
	Symbol *pNewSym = 0;

	if(stype == SYMBOL_MAP)
	{
		pNewSym = this->clone();
	}
	if(stype == SYMBOL_STRING)
	{
		SymbolString *pNewSymS = new SymbolString();
		pNewSymS->SetName(this->GetName());
		pNewSymS->SetVal(print());

		pNewSym = pNewSymS;
	}
	else
		log_err("Cannot convert map to other type.");

	return pNewSym;
}

t_string SymbolMap::print() const
{
	t_ostringstream ostr;

	ostr << T_STR"[";
	unsigned int iIter = 0;
	for(const t_map::value_type& val : m_map)
	{
		const Symbol* pSym = val.second;
		if(val.first.strKey == "")
			ostr << T_STR"#" << std::hex << val.first.key << std::dec;
		else
			ostr << val.first.strKey;
		ostr << ": " << (val.second ? val.second->print() : T_STR"");

		if(iIter < m_map.size()-1)
			ostr << T_STR", ";

		++iIter;
	}
	ostr << T_STR"]";

	return ostr.str();
}

Symbol* SymbolMap::clone() const
{
	//G_COUT << "SymbolMap::clone" << std::endl;
	SymbolMap *pSym = new SymbolMap;

	for(const t_map::value_type& val : m_map)
		pSym->m_map.insert(t_map::value_type(val.first, val.second->clone()));

	pSym->UpdateIndices();
	return pSym;
}

void SymbolMap::assign(Symbol *pSym)
{
	//G_COUT << "SymbolMap::assign" << std::endl;

	SymbolMap *pOther = (SymbolMap*)pSym->ToType(GetType());
	this->m_map = pOther->m_map;
}

void SymbolMap::UpdateIndex(const t_map::key_type& key)
{
	t_map::iterator iter = m_map.find(key);
	if(iter != m_map.end() && iter->second)
	{
		//G_COUT << "updating index for " << strKey << std::endl;

		iter->second->SetMapPtr(this);
		iter->second->SetMapKey(iter->first);
	}
}

void SymbolMap::UpdateIndices()
{
	for(const t_map::value_type& val : m_map)
	{
		//G_COUT << "updating index for " << val.first << std::endl;

		val.second->SetMapPtr(this);
		val.second->SetMapKey(val.first);
	}
}

t_string SymbolMap::GetStringVal(const SymbolMapKey& key, bool *pbHasVal) const
{
	if(pbHasVal) *pbHasVal = 0;

	t_map::const_iterator iter = m_map.find(key);
	if(iter == m_map.end())
		return T_STR"";

	if(pbHasVal) *pbHasVal = 1;

	Symbol *pSym = iter->second;
	if(!pSym)
		return T_STR"";

	if(pbHasVal) *pbHasVal = 1;
	return pSym->print();
}

t_int SymbolMap::GetIntVal(const SymbolMapKey& key, bool *pbHasVal) const
{
	if(pbHasVal) *pbHasVal = 0;

	t_map::const_iterator iter = m_map.find(key);
	if(iter == m_map.end())
		return 0;

	if(pbHasVal) *pbHasVal = 1;

	const Symbol *pSym = iter->second;
	if(!pSym)
		return 0;

	if(pbHasVal) *pbHasVal = 1;
		return pSym->GetValInt();
}

std::size_t SymbolMap::hash() const
{
	static const std::size_t iMapSeed = SYMBOL_MAP;	// unique seed

	std::size_t iSeed = iMapSeed;
	for(t_map::const_iterator iter=m_map.begin(); iter!=m_map.end(); ++iter)
	{
		std::size_t iHshKey = iter->first.key;
		boost::hash_combine(iSeed, iHshKey);

		std::size_t iHshVal = iter->second->hash();
		boost::hash_combine(iSeed, iHshVal);
	}
	return iSeed;
}

//--------------------------------------------------------------------------------


SymbolTable::SymbolTable()
{}

SymbolTable::~SymbolTable()
{
	std::set<Symbol*> m_setDeleted;

	for(t_syms::iterator iter=m_syms.begin(); iter!=m_syms.end(); ++iter)
	{
		Symbol *pSym = iter->second;
		if(pSym && m_setDeleted.find(pSym)!=m_setDeleted.end())
		{
			m_setDeleted.insert(pSym);

			delete pSym;
			iter->second = 0;
		}
	}
}


void SymbolTable::print() const
{
	for(const auto& val : m_syms)
	{
		Symbol *pSym = val.second;
		t_string strSym;
		if(pSym)
		{
			t_string strType = pSym->GetTypeName();
			strSym = pSym->print() + T_STR" (" + strType + T_STR")";
		}
		else
			strSym = T_STR"<null>";

		log_info(val.first, " = ", strSym);
	}
}

Symbol* SymbolTable::GetSymbol(const t_string& strKey)
{
	auto sym = m_syms.find(strKey);
	if(sym == m_syms.end())
		return 0;
	return sym->second;
}

void SymbolTable::InsertSymbol(const t_string& strKey, Symbol *pSym)
{
	RemoveSymbol(strKey);
	if(pSym)
		pSym->SetRval(0);
	m_syms[strKey] = pSym;
}

void SymbolTable::RemoveSymbol(const t_string& strKey)
{
	t_syms::iterator iter = m_syms.find(strKey);
	if(iter != m_syms.end())
	{
		if(iter->second) delete iter->second;
		m_syms.erase(iter);
	}
}

void SymbolTable::RemoveSymbolNoDelete(const t_string& strKey)
{
	m_syms.erase(strKey);
}

bool SymbolTable::IsPtrInMap(const Symbol* pSym) const
{
	for(auto entry : m_syms)
	{
		const Symbol *pSymInMap = entry.second;
		if(pSymInMap == pSym)
			return 1;
	}

	return 0;
}


// --------------------------------------------------------------------------------


bool is_vec(const Symbol* pSym)
{
	if(!pSym)
		return false;
	if(pSym->GetType() != SYMBOL_ARRAY)
		return false;

	const SymbolArray* pSymArr = (SymbolArray*)pSym;
	for(const Symbol* pSymInArr : pSymArr->GetArr())
	{
		if(!pSymInArr->IsScalar())
			return false;
	}
	return true;
}

bool is_mat(const Symbol* pSym, unsigned int *piNumCols, unsigned int *piNumRows)
{
	if(!pSym)
		return false;
	if(pSym->GetType() != SYMBOL_ARRAY)
		return false;

	const SymbolArray* pSymArr = (SymbolArray*)pSym;
	if(piNumRows) *piNumRows = pSymArr->GetArr().size();

	unsigned int iVecSize = 0;
	bool bHasSize = 0;
	for(const Symbol* pSymInArr : pSymArr->GetArr())
	{
		if(!is_vec(pSymInArr))
			return false;

		unsigned int iSize = ((SymbolArray*)pSymInArr)->GetArr().size();
		if(!bHasSize)
		{
			iVecSize = iSize;
			bHasSize = 1;

			if(piNumCols) *piNumCols = iVecSize;
		}

		// element vectors have to be of the same size
		if(iSize != iVecSize)
			return false;
	}

	return true;
}


// --------------------------------------------------------------------------------

void safe_delete(Symbol *&pSym, const SymbolTable* pSymTab, const SymbolTable* pSymTabGlob)
{
	if(!pSym) return;

	// don't delete constants
	if(pSym->IsConst())
		return;

	// don't delete array or map members
	if(pSym->GetArrPtr() || pSym->GetMapPtr())
		return;

	// don't delete symbols in table
	bool bIsInTable = pSymTab->IsPtrInMap(pSym);
	bool bIsInGlobTable = pSymTabGlob->IsPtrInMap(pSym);
	if(!bIsInTable && !bIsInGlobTable)
	{
		//log_debug("safe_deleting ", (void*)pSym, ", type: ", pSym->GetType(), ", val: ", pSym->print());
		//log_backtrace();

		delete pSym;
		pSym = 0;
	}
}

bool is_tmp_sym(const Symbol* pSym)
{
	const bool bIsMember = pSym->GetArrPtr() || pSym->GetMapPtr();
	if(bIsMember)
		return false;

	const bool bNoConst = !pSym->IsConst();
	//const bool bNoIdent = (pSym->GetName().size()==0);
	return bNoConst && pSym->IsRval() /*&& bNoIdent*/;
}

bool clone_if_needed(Symbol *pSym, Symbol*& pClone)
{
	if(!pSym)
	{
		pClone = 0;
		return 0;
	}

	bool bNeedClone = !is_tmp_sym(pSym);

	if(bNeedClone)
		pClone = pSym->clone();
	else
		pClone = pSym;

	//log_debug("Cloned: ", bNeedClone);
	return bNeedClone;
}

Symbol* recycle_or_alloc(const std::initializer_list<const Symbol*>& lstSyms, bool bAlwaysAlloc)
{
	if(!lstSyms.size())
		return 0;

	if(!bAlwaysAlloc)
	{
		Symbol *pSymTmp = 0;

		// find recyclable symbol (all symbols have to be of the same type!)
		for(const Symbol* pSym : lstSyms)
		{
			if(is_tmp_sym(pSym))
			{
				//log_debug("Recycling ", pSym, ", ", pSym->GetType(), ": ", pSym->GetIdent(), " = ", pSym->print());
				pSymTmp = const_cast<Symbol*>(pSym);
				pSymTmp->ClearIndices();
				pSymTmp->SetRval(1);
				pSymTmp->SetConst(0);
				break;
			}
		}

		if(pSymTmp)
			return pSymTmp;
	}

	// allocate empty symbol of the same type
	return (*lstSyms.begin())->alloc();
}
