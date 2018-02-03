
#include "TargetValue.h"
#include "Debug.h"
#include <iostream>
#include <fstream>
#include <sstream>

const ompd_callbacks_t *TValue::callbacks=NULL;
ompd_target_type_sizes_t TValue::type_sizes;

inline int ompd_sizeof(ompd_target_prim_types_t t)
{
    return (((int*)&TValue::type_sizes)[(int)t]);
}

TType& TTypeFactory::getType(
        ompd_address_space_context_t *context, 
        const char * typeName, 
        ompd_taddr_t segment
)
{
    TType empty(true);

    if ( ttypes.find(context) == ttypes.end() ) {
        std::map<const char*, TType> empty;
        ttypes[context] = empty;
    }

    auto t = ttypes.find(context);
    auto i = t->second.find(typeName);
    if ( i == t->second.end() )
        i = t->second.insert(i, std::make_pair(typeName, TType(context, typeName, segment)));
    else
        i->second.context = context;

    return i->second;
}


TType::TType(ompd_address_space_context_t *_context, 
        const char* _typeName, 
        ompd_taddr_t _segment) : typeSize(0), fieldOffsets(), descSegment(_segment), 
      typeName(_typeName), context(_context), isvoid(false)
{
}

ompd_rc_t TType::getSize(ompd_size_t * size)
{
  ompd_rc_t ret = ompd_rc_ok;
  if(typeSize==0)
  {
    ompd_address_t symbolAddr;
    ompd_size_t tmpSize;
    std::stringstream ss;
    ss << "ompd_sizeof__" << typeName;
    ret = TValue::callbacks->tsymbol_addr(context, NULL, ss.str().c_str(), &symbolAddr);
    if (ret!=ompd_rc_ok){
      dout << "missing symbol " 
          << ss.str() 
          << " add this to ompd-specific.h:\nOMPD_SIZEOF(" << typeName << ") \\" 
          << std::endl;
      return ret;
    }
    symbolAddr.segment = descSegment;

    ret = TValue::callbacks->read_tmemory(context, NULL, symbolAddr,
      1 * ompd_sizeof(ompd_type_long_long), &(tmpSize));
    if (ret!=ompd_rc_ok)
      return ret;
    ret = TValue::callbacks->target_to_host(context, &tmpSize, 
      ompd_sizeof(ompd_type_long_long), 1, &(typeSize));
  }
  *size = typeSize;
  return ret;
}

ompd_rc_t TType::getBitfieldMask (const char* fieldName, uint64_t * bitfieldmask)
{
  ompd_rc_t ret = ompd_rc_ok;
  auto i = bitfieldMasks.find(fieldName);
  if ( i == bitfieldMasks.end() )
  {
    uint64_t tmpMask, bitfieldMask;
    ompd_address_t symbolAddr;
//    ret = callbacks->ttype_offset(context, &OmpdTypeHandle, fieldName, &fieldOffset);
    std::stringstream ss;
    ss << "ompd_bitfield__" << typeName << "__" << fieldName;
    ret = TValue::callbacks->tsymbol_addr(context, NULL, ss.str().c_str(), &symbolAddr);
    if (ret!=ompd_rc_ok){
      dout << "missing symbol " << ss.str() << " add this to ompd-specific.h:\nOMPD_BITFIELD(" << typeName << "," << fieldName << ") \\" << std::endl;
      return ret;
    }
    symbolAddr.segment = descSegment;

    ret = TValue::callbacks->read_tmemory(context, NULL, symbolAddr,
      1 * ompd_sizeof( ompd_type_long_long), &(tmpMask));
    if (ret!=ompd_rc_ok)
      return ret;
    ret = TValue::callbacks->target_to_host(context, &(tmpMask), 
      ompd_sizeof(ompd_type_long_long), 1, &(bitfieldMask));
    if (ret!=ompd_rc_ok){
      return ret;
    }
    i = bitfieldMasks.insert(i,std::make_pair(fieldName, bitfieldMask));
  }
  *bitfieldmask = i->second;
  return ret;
}

ompd_rc_t TType::getElementOffset(const char* fieldName, ompd_size_t * offset)
{
  ompd_rc_t ret = ompd_rc_ok;
  auto i = fieldOffsets.find(fieldName);
  if ( i == fieldOffsets.end() )
  {
    ompd_size_t tmpOffset, fieldOffset;
    ompd_address_t symbolAddr;
//    ret = callbacks->ttype_offset(context, &OmpdTypeHandle, fieldName, &fieldOffset);
    std::stringstream ss;
    ss << "ompd_access__" << typeName << "__" << fieldName;
    ret = TValue::callbacks->tsymbol_addr(context, NULL, ss.str().c_str(), &symbolAddr);
    if (ret!=ompd_rc_ok){
      dout << "missing symbol " << ss.str() << " add this to ompd-specific.h:\nOMPD_ACCESS(" << typeName << "," << fieldName << ") \\" << std::endl;
      return ret;
    }
    symbolAddr.segment = descSegment;

    ret = TValue::callbacks->read_tmemory(context, NULL, symbolAddr,
      1* ompd_sizeof(ompd_type_long_long), &(tmpOffset));
    if (ret!=ompd_rc_ok)
      return ret;
    ret = TValue::callbacks->target_to_host(context, &(tmpOffset), 
      ompd_sizeof(ompd_type_long_long), 1, &fieldOffset);
    if (ret!=ompd_rc_ok){
      return ret;
    }
    i = fieldOffsets.insert(i,std::make_pair(fieldName, fieldOffset));
  }
  *offset = i->second;
  return ret;
}

ompd_rc_t TType::getElementSize(const char* fieldName, ompd_size_t * size)
{
  ompd_rc_t ret = ompd_rc_ok;
  auto i = fieldSizes.find(fieldName);
  if ( i == fieldSizes.end() )
  {
    ompd_size_t tmpOffset, fieldSize;
    ompd_address_t symbolAddr;
//    ret = callbacks->ttype_offset(context, &OmpdTypeHandle, fieldName, &fieldOffset);
    std::stringstream ss;
    ss << "ompd_sizeof__" << typeName << "__" << fieldName;
    ret = TValue::callbacks->tsymbol_addr(context, NULL, ss.str().c_str(), &symbolAddr);
    if (ret!=ompd_rc_ok){
      dout << "missing symbol " << ss.str() << " add this to ompd-specific.h:\nOMPD_ACCESS(" << typeName << "," << fieldName << ") \\" << std::endl;
      return ret;
    }
    symbolAddr.segment = descSegment;

    ret = TValue::callbacks->read_tmemory(context, NULL, symbolAddr,
      1* ompd_sizeof(ompd_type_long_long), &(tmpOffset));
    if (ret!=ompd_rc_ok)
      return ret;
    ret = TValue::callbacks->target_to_host(context, &tmpOffset, 
      ompd_sizeof(ompd_type_long_long), 1, &fieldSize);
    if (ret!=ompd_rc_ok){
      return ret;
    }
    i = fieldSizes.insert(i,std::make_pair(fieldName, fieldSize));
  }
  *size = i->second;
  return ret;
}



//class VoidType : TType
//{
//public:
//  virtual bool isVoid(){return true;}
//}


//static VoidType nullType();

//class TValue
//{
//protected:
//  TType& type = nullType;
//  int pointerLevel;
//  const char* valueName;
//  ompd_address_space_context_t *context;
//  ompd_address_t symbolAddr;
//public:  
//  TValue(ompd_address_space_context_t *context, const char* valueName);
//  TValue& cast(const char* typeName);
//  TValue& cast(const char* typeName, int pointerLevel);
//  TValue& castBase(ompd_target_prim_types_t baseType);
//  TValue access(const char* fieldName) const;
//  TValue getArrayElement(int elemNumber) const;
//};


TValue::TValue(ompd_address_space_context_t *_context, ompd_thread_context_t *_tcontext, const char* _valueName, ompd_taddr_t segment)
: errorState(ompd_rc_ok), type(&nullType), pointerLevel(0), /*valueName(_valueName),*/ context(_context), tcontext(_tcontext), fieldSize(0)
{
  errorState.errorCode = callbacks->tsymbol_addr(context, tcontext, _valueName, &symbolAddr);
  symbolAddr.segment = segment;
//  assert((ret==ompd_rc_ok) && "Callback call failed");
}

TValue::TValue(ompd_address_space_context_t *_context, ompd_thread_context_t *_tcontext, ompd_address_t addr)
: errorState(ompd_rc_ok), type(&nullType), pointerLevel(0), context(_context), tcontext(_tcontext), symbolAddr(addr), fieldSize(0)
{
  if(addr.address==0)
    errorState.errorCode = ompd_rc_bad_input;
}

// TValue::TValue(ompd_address_space_context_t *_context, ompd_thread_context_t *_tcontext, const struct ompd_handle* handle)
// : errorState(ompd_rc_ok), type(&nullType), pointerLevel(0), context(_context), tcontext(_tcontext), symbolAddr(handle->th)
// {
// }

TValue& TValue::cast(const char* typeName)
{
  if (gotError())
    return *this;
  type = &tf.getType(context, typeName, symbolAddr.segment);
  pointerLevel = 0;
  assert(!type->isVoid() && "cast to invalid type failed");
  return *this;
}

TValue& TValue::cast(const char* typeName, int _pointerLevel, ompd_taddr_t segment)
{
  if (gotError())
    return *this;
  type = &tf.getType(context, typeName, symbolAddr.segment);
  pointerLevel = _pointerLevel;
  symbolAddr.segment = segment;
  assert(!type->isVoid() && "cast to invalid type failed");
  return *this;
}

TValue TValue::dereference() const
{
  if (gotError())
    return *this;
  ompd_address_t tmpAddr;
  assert(!type->isVoid() && "cannot work with void");
  assert(pointerLevel > 0 && "cannot dereference non-pointer");
  TValue ret=*this;
  ret.pointerLevel--;
  ret.errorState.errorCode = callbacks->read_tmemory(context, tcontext, symbolAddr,
      1 * ompd_sizeof( ompd_type_pointer), &(tmpAddr.address));
  if (ret.errorState.errorCode!=ompd_rc_ok)
      return ret;

  ret.errorState.errorCode = callbacks->target_to_host(context, &(tmpAddr.address), 
      ompd_sizeof(ompd_type_pointer), 1, &(ret.symbolAddr.address));
  if (ret.errorState.errorCode!=ompd_rc_ok){
    return ret;
  }
  if (ret.symbolAddr.address==0)
    ret.errorState.errorCode = ompd_rc_unsupported;
  return ret;
}

ompd_rc_t TValue::getAddress(ompd_address_t * addr) const
{
  *addr = symbolAddr;
  if (symbolAddr.address==0)
    return ompd_rc_unsupported;
  return errorState.errorCode;
}

ompd_rc_t TValue::getRawValue(void* buf, int count)
{
  if( errorState.errorCode != ompd_rc_ok )
    return errorState.errorCode;
  ompd_size_t size;
  errorState.errorCode = type->getSize(&size);
  if( errorState.errorCode != ompd_rc_ok )
    return errorState.errorCode;

  errorState.errorCode = callbacks->read_tmemory(context, tcontext, symbolAddr,
      size, buf);
  return errorState.errorCode;
}

// ompd_rc_t TValue::getAddress(struct ompd_handle* handle) const
// {
//   handle->th = symbolAddr;
//   return errorState.errorCode;
// }

TBaseValue TValue::castBase(const char* varName)
{  
  ompd_size_t size;
  errorState.errorCode = tf.getType(context, varName, symbolAddr.segment).getSize(&size);
  return TBaseValue(*this, size);
}

TBaseValue TValue::castBase() const
{  
  return TBaseValue(*this, fieldSize);
}


TBaseValue TValue::castBase(ompd_target_prim_types_t baseType) const
{
  return TBaseValue(*this, baseType);
}

TValue TValue::access(const char* fieldName) const
{
  if (gotError())
    return *this;
  TValue ret = *this;
  assert (pointerLevel<2 && "access to field element of pointer array failed");
  if (pointerLevel==1) // -> operator
    ret = ret.dereference();
    // we use *this for . operator
  ompd_size_t offset;
  ret.errorState.errorCode = type->getElementOffset(fieldName, &offset);
  ret.errorState.errorCode = type->getElementSize(fieldName, &(ret.fieldSize));
  ret.symbolAddr.address += offset;
  
  return ret;
}

ompd_rc_t TValue::check(const char* bitfieldName, ompd_tword_t* isSet) const
{
  if (gotError())
    return getError();
  int bitfield;
  uint64_t bitfieldmask;
  ompd_rc_t ret = this->castBase(ompd_type_int).getValue(&bitfield,1);
  if (ret != ompd_rc_ok)
    return ret;
  ret = type->getBitfieldMask(bitfieldName, &bitfieldmask);
  *isSet = ((bitfield & bitfieldmask)!=0);
  return ret;
}

TValue TValue::getArrayElement(int elemNumber) const
{
  if (gotError())
    return *this;
  TValue ret=dereference();
  if (ret.pointerLevel==0)
  {
    ompd_size_t size;
    ret.errorState.errorCode = type->getSize(&size);
    ret.symbolAddr.address += elemNumber * size;
  }
  else
  {
    ret.symbolAddr.address += elemNumber * type_sizes.sizeof_pointer;
  }
  return ret;
}

TBaseValue::TBaseValue(const TValue& _tvalue, ompd_target_prim_types_t _baseType): TValue(_tvalue), baseTypeSize(ompd_sizeof(_baseType)){}
TBaseValue::TBaseValue(const TValue& _tvalue, ompd_size_t _baseTypeSize): TValue(_tvalue), baseTypeSize(_baseTypeSize){}

ompd_rc_t TBaseValue::getValue(void* buf, int count)
{
  if( errorState.errorCode != ompd_rc_ok )
    return errorState.errorCode;
  errorState.errorCode = callbacks->read_tmemory(context, tcontext, symbolAddr,
      count *baseTypeSize, buf);
  if (errorState.errorCode!=ompd_rc_ok)
      return errorState.errorCode;
  errorState.errorCode = callbacks->target_to_host(context, buf, 
      baseTypeSize, count, buf);
  return errorState.errorCode;
}

// ompd_rc_t TBaseValue::getValue(struct ompd_handle* buf, int count)
// {
//   if( errorState.errorCode != ompd_rc_ok )
//     return errorState.errorCode;
//   errorState.errorCode = callbacks->read_tmemory(context, tcontext, symbolAddr,
//       count, baseType, &(buf->th));
//   assert((errorState.errorCode == ompd_rc_ok) && "Callback call failed");
//   return errorState.errorCode;
// }


