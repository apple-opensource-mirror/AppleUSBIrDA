/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
    File:       CList.c

    Contains:   Implementation of the CList class

*/

#include "CList.h"
#include "CListIterator.h"
#include "CItemComparer.h"
#include "IrDALog.h"

#if (hasTracing > 0 && hasCListTracing > 0)

enum TraceCodes {
    kLogNew = 1,
    kLogFree,
    kLogInit
};

static
EventTraceCauseDesc TraceEvents[] = {
    {kLogNew,           "CList: new, obj="},
    {kLogFree,          "CList: free, obj="},
    {kLogInit,          "CList: init, obj="}
};
#define XTRACE(x, y, z) IrDALogAdd( x, y, z, TraceEvents, true )
#else
#define XTRACE(x, y, z) ((void) 0)
#endif


//--------------------------------------------------------------------------------
#define super CDynamicArray
    OSDefineMetaClassAndStructors(CList, CDynamicArray);
//--------------------------------------------------------------------------------


CList *
CList::cList(long chunkSize)
{
    CList *obj = new CList;
    
    XTRACE(kLogNew, (int)obj >> 16, (short)obj);
    
    if (obj && !obj->init(chunkSize)) {
	obj->release();
	obj = nil;
    }
    return obj;
}

Boolean CList::init(ArrayIndex size)
{
    XTRACE(kLogInit, (int)this >> 16, (short)this);
    
    return super::init(kDefaultElementSize, size);
}

void CList::free(void)
{
    XTRACE(kLogFree, (int)this >> 16, (short)this);
    
    super::free();
}

//----------------------------------------------------------------------------
//      CList::At
//----------------------------------------------------------------------------
void* CList::At(ArrayIndex index)
{
    ULong* itemPtr = (ULong*) SafeElementPtrAt(index);
    return (itemPtr == nil) ? nil : (void*) (*itemPtr);

} // CList::At


//----------------------------------------------------------------------------
//      CList::InsertAt
//----------------------------------------------------------------------------
IrDAErr CList::InsertAt(ArrayIndex index, void* item)
{
    ULong data = (ULong) item;

    return InsertElementsBefore(index, &data, 1);

} // CList::InsertBefore


//----------------------------------------------------------------------------
//      CList::Remove
//----------------------------------------------------------------------------
IrDAErr CList::Remove(void* item)
{
    IrDAErr result;
    ArrayIndex index = GetIdentityIndex(item);

    if (index != kEmptyIndex)
	result = RemoveAt(index);
    else
	{
	result = errRangeCheck;
	//XPRINT(("CList::Remove: can't find item in list\n"));
	}

    return result;

} // CList::Remove


//----------------------------------------------------------------------------
//      CList::InsertUnique
//----------------------------------------------------------------------------
Boolean CList::InsertUnique(void* add)
    {
    Boolean result = !Contains(add);
    if (result)
	{
	IrDAErr ignored = InsertLast(add);
	XASSERTNOT(ignored);
	}
    return result;
    }


//----------------------------------------------------------------------------
//      CList::Replace
//----------------------------------------------------------------------------
IrDAErr CList::Replace(void* oldItem, void* newItem)
{
    IrDAErr result;
    ArrayIndex index = GetIdentityIndex(oldItem);

    if (index != kEmptyIndex)
	result = ReplaceAt(index, newItem);
    else
	{
	result = errRangeCheck;
	//XPRINT(("CList::Replace: can't find oldItem in list\n"));
	}

    return result;

} // CList::Replace


//----------------------------------------------------------------------------
//      CList::ReplaceAt
//----------------------------------------------------------------------------
IrDAErr CList::ReplaceAt(ArrayIndex index, void* newItem)
{
    ULong data = (ULong) newItem;

    return ReplaceElementsAt(index, &data, 1);
}

//----------------------------------------------------------------------------
//      CList::GetIdentityIndex
//----------------------------------------------------------------------------
ArrayIndex CList::GetIdentityIndex(void* item)
{
    ArrayIndex index = -1;
    CItemComparer *test =  CItemComparer::cItemComparer(item, nil);     // default comparer tests for identity

    require(test, Fail);
    
    (void) Search(test, index);

    test->release();
    
Fail:
    return index;

} // CList::GetIdentityIndex


//----------------------------------------------------------------------------
//      CList::Search
//----------------------------------------------------------------------------
void* CList::Search(CItemComparer* test, ArrayIndex& index)
/*
    Performs a linear search on the list.
    Returns the index and value of the item for which
    CItemTester::TestItem returns kItemEqualCriteria,
    or kEmptyIndex and nil respectively.
*/
{
    CListIterator *iter = CListIterator::cListIterator(this);
    index = kEmptyIndex;
    void* result = nil;
    void* item = nil;

    require(iter, Fail);
    
    for (item = iter->FirstItem(); iter->More(); item = iter->NextItem())
	if (test->TestItem(item) == kItemEqualCriteria)
	    {
	    result = item;
	    index = iter->fCurrentIndex;
	    break;
	    }

    iter->release();
    
Fail:
    return result;

} // CList::Search

