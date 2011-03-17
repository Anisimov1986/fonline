#include "StdAfx.h"
#include "Item.h"
#include "ItemManager.h"

#ifdef FONLINE_SERVER
#include "MapManager.h"
#include "CritterManager.h"
#include "AI.h"
#endif

const char* ItemEventFuncName[ITEM_EVENT_MAX]=
{
	{"void %s(Item&,bool)"}, // ITEM_EVENT_FINISH
	{"bool %s(Item&,Critter&,Critter&)"}, // ITEM_EVENT_ATTACK
	{"bool %s(Item&,Critter&,Critter@,Item@,Scenery@)"}, // ITEM_EVENT_USE
	{"bool %s(Item&,Critter&,Item@)"}, // ITEM_EVENT_USE_ON_ME
	{"bool %s(Item&,Critter&,int)"}, // ITEM_EVENT_SKILL
	{"void %s(Item&,Critter&)"}, // ITEM_EVENT_DROP
	{"void %s(Item&,Critter&,uint8)"}, // ITEM_EVENT_MOVE
	{"void %s(Item&,Critter&,bool,uint8)"}, // ITEM_EVENT_WALK
};

// Item default send data mask
char Item::ItemData::SendMask[ITEM_DATA_MASK_MAX][92]=
{
	// SortValue Info Reserved0 PicMapHash   PicInvHash   AnimWaitBase AnimStay[2] AnimShow[2] AnimHide[2] Flags        Rate LightIntensity LightDistance LightFlags LightColor   ScriptId TrapValue Count        Cost         ScriptValues[10]                                                                 Shared data 8 bytes
	// ITEM_DATA_MASK_CHOSEN                                                                                           ITEM_DATA_MASK_CHOSEN                                                                                                ITEM_DATA_MASK_CHOSEN                                                                                              
	{  -1,-1,     -1,    0,     -1,-1,-1,-1, -1,-1,-1,-1,      0,0,       0,0,        0,0,        0,0,     -1,-1,-1,-1,  -1,       -1,            -1,        -1,     -1,-1,-1,-1,   0,0,     0,0,    -1,-1,-1,-1, -1,-1,-1,-1, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, -1,-1,-1,-1,-1,-1,-1,-1, },
	// ITEM_DATA_MASK_CRITTER                                                                                          ITEM_DATA_MASK_CRITTER                                                                                               ITEM_DATA_MASK_CRITTER                                                                        
	{    0,0,     -1,    0,         0,0,0,0,     0,0,0,0,      0,0,       0,0,        0,0,        0,0,     -1,-1,-1,-1,  -1,       -1,            -1,        -1,     -1,-1,-1,-1,   0,0,     0,0,        0,0,0,0,     0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,         0,0,0,0,0,0,0,0, },
	// ITEM_DATA_MASK_CRITTER_EXT                                                                                      ITEM_DATA_MASK_CRITTER_EXT                                                                                           ITEM_DATA_MASK_CRITTER_EXT                                                                    
	{    0,0,     -1,    0,         0,0,0,0,     0,0,0,0,      0,0,       0,0,        0,0,        0,0,     -1,-1,-1,-1,  -1,       -1,            -1,        -1,     -1,-1,-1,-1,   0,0,     0,0,    -1,-1,-1,-1,     0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, -1,-1,-1,-1,-1,-1,-1,-1, },
	// ITEM_DATA_MASK_CONTAINER                                                                                        ITEM_DATA_MASK_CONTAINER                                                                                             ITEM_DATA_MASK_CONTAINER                                                                      
	{  -1,-1,     -1,    0,         0,0,0,0, -1,-1,-1,-1,      0,0,       0,0,        0,0,        0,0,     -1,-1,-1,-1,  -1,       -1,            -1,        -1,     -1,-1,-1,-1,   0,0,     0,0,    -1,-1,-1,-1, -1,-1,-1,-1, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, -1,-1,-1,-1,-1,-1,-1,-1, },
	// ITEM_DATA_MASK_MAP                                                                                              ITEM_DATA_MASK_MAP                                                                                                   ITEM_DATA_MASK_MAP                                                                            
	{  -1,-1,     -1,    0,     -1,-1,-1,-1,     0,0,0,0,    -1,-1,     -1,-1,      -1,-1,      -1,-1,     -1,-1,-1,-1,  -1,       -1,            -1,        -1,     -1,-1,-1,-1,   0,0,     0,0,        0,0,0,0,     0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,         0,0,0,0,0,0,0,0, },
};

/************************************************************************/
/* Item                                                                 */
/************************************************************************/

void Item::Init(ProtoItem* proto)
{
	if(!proto)
	{
		WriteLog(__FUNCTION__" - Proto is null ptr.\n");
		return;
	}

#ifdef FONLINE_SERVER
	ViewByCritter=NULL;
	ViewPlaceOnMap=false;
	ChildItems=NULL;
	From=0;
#endif

	ZeroMemory(&Data,sizeof(Data));
	Proto=proto;
	Accessory=ITEM_ACCESSORY_NONE;
	Data.SortValue=0x7FFF;
	Data.AnimWaitBase=Proto->AnimWaitBase;
	Data.AnimStay[0]=Proto->AnimStay[0];
	Data.AnimStay[1]=Proto->AnimStay[1];
	Data.AnimShow[0]=Proto->AnimShow[0];
	Data.AnimShow[1]=Proto->AnimShow[1];
	Data.AnimHide[0]=Proto->AnimHide[0];
	Data.AnimHide[1]=Proto->AnimHide[1];
	Data.Flags=Proto->Flags;
	Data.Indicator=Proto->IndicatorStart;

	for(int i=0;i<ITEM_MAX_SCRIPT_VALUES;i++)
		Data.ScriptValues[i]=Proto->StartValue[i];

	switch(GetType())
	{
	case ITEM_TYPE_WEAPON:
		Data.TechInfo.AmmoCount=Proto->Weapon_MaxAmmoCount;
		Data.TechInfo.AmmoPid=Proto->Weapon_DefaultAmmoPid;
		break;
	case ITEM_TYPE_DOOR:
		SETFLAG(Data.Flags,ITEM_GAG);
		if(!Proto->Door_NoBlockMove) UNSETFLAG(Data.Flags,ITEM_NO_BLOCK);
		if(!Proto->Door_NoBlockShoot) UNSETFLAG(Data.Flags,ITEM_SHOOT_THRU);
		if(!Proto->Door_NoBlockLight) UNSETFLAG(Data.Flags,ITEM_LIGHT_THRU);
		Data.Locker.Condition=Proto->Locker_Condition;
		break;
	case  ITEM_TYPE_CONTAINER:
		Data.Locker.Condition=Proto->Locker_Condition;
		break;
	default:
		break;
	}

	if(IsRadio())
	{
		Data.Radio.Channel=Proto->RadioChannel;
		Data.Radio.Flags=Proto->RadioFlags;
		Data.Radio.BroadcastSend=Proto->RadioBroadcastSend;
		Data.Radio.BroadcastRecv=Proto->RadioBroadcastRecv;
	}

	if(IsHolodisk())
	{
		Data.Holodisk.Number=Proto->HolodiskNum;
#ifdef FONLINE_SERVER
		if(!Data.Holodisk.Number) Data.Holodisk.Number=Random(1,42);
#endif
	}

#ifdef FONLINE_CLIENT
	Lexems="";
	LexemsRefCounter=0x80000000;
#endif
}

Item* Item::Clone()
{
	Item* clone=new Item();
	clone->Id=Id;
	clone->Proto=Proto;
	clone->Accessory=Accessory;
	clone->ACC_BUFFER=ACC_BUFFER;
	clone->Data=Data;

#ifdef FONLINE_SERVER
	clone->ViewByCritter=NULL;
	clone->ViewPlaceOnMap=false;
	clone->ChildItems=NULL;
	clone->PLexems=NULL;
#endif
#ifdef FONLINE_CLIENT
	clone->Lexems=Lexems;
	clone->LexemsRefCounter=LexemsRefCounter;
#endif

	return clone;
}


#ifdef FONLINE_SERVER
void Item::FullClear()
{
	IsNotValid=true;
	Accessory=0x20+GetType();

	if(IsContainer() && ChildItems)
	{
		MEMORY_PROCESS(MEMORY_ITEM,-(int)sizeof(ItemPtrMap));

		ItemPtrVec del_items=*ChildItems;
		ChildItems->clear();
		SAFEDEL(ChildItems);

		for(ItemPtrVecIt it=del_items.begin(),end=del_items.end();it!=end;++it)
		{
			Item* item=*it;
			SYNC_LOCK(item);
			item->Accessory=0xB1;
			ItemMngr.ItemToGarbage(item);
		}
	}
}

bool Item::ParseScript(const char* script, bool first_time)
{
	if(script && script[0])
	{
		DWORD func_num=Script::GetScriptFuncNum(script,"void %s(Item&,bool)");
		if(!func_num)
		{
			WriteLog(__FUNCTION__" - Script<%s> bind fail, item pid<%u>.\n",script,GetProtoId());
			return false;
		}
		Data.ScriptId=func_num;
	}

	if(Data.ScriptId && Script::PrepareContext(Script::GetScriptFuncBindId(Data.ScriptId),CALL_FUNC_STR,Str::Format("Item id<%u>, pid<%u>",GetId(),GetProtoId())))
	{
		Script::SetArgObject(this);
		Script::SetArgBool(first_time);
		Script::RunPrepared();
	}
	return true;
}

bool Item::PrepareScriptFunc(int num_scr_func)
{
	if(num_scr_func>=ITEM_EVENT_MAX) return false;
	if(FuncId[num_scr_func]<=0) return false;
	return Script::PrepareContext(FuncId[num_scr_func],CALL_FUNC_STR,Str::Format("Item id<%u>, pid<%u>",GetId(),GetProtoId()));
}

void Item::EventFinish(bool deleted)
{
	if(PrepareScriptFunc(ITEM_EVENT_FINISH))
	{
		Script::SetArgObject(this);
		Script::SetArgBool(deleted);
		Script::RunPrepared();
	}
}

bool Item::EventAttack(Critter* cr, Critter* target)
{
	bool result=false;
	if(PrepareScriptFunc(ITEM_EVENT_ATTACK))
	{
		Script::SetArgObject(this);
		Script::SetArgObject(cr);
		Script::SetArgObject(target);
		if(Script::RunPrepared()) result=Script::GetReturnedBool();
	}
	return result;
}

bool Item::EventUse(Critter* cr, Critter* on_critter, Item* on_item, MapObject* on_scenery)
{
	bool result=false;
	if(PrepareScriptFunc(ITEM_EVENT_USE))
	{
		Script::SetArgObject(this);
		Script::SetArgObject(cr);
		Script::SetArgObject(on_critter);
		Script::SetArgObject(on_item);
		Script::SetArgObject(on_scenery);
		if(Script::RunPrepared()) result=Script::GetReturnedBool();
	}
	return result;
}

bool Item::EventUseOnMe(Critter* cr, Item* used_item)
{
	bool result=false;
	if(PrepareScriptFunc(ITEM_EVENT_USE_ON_ME))
	{
		Script::SetArgObject(this);
		Script::SetArgObject(cr);
		Script::SetArgObject(used_item);
		if(Script::RunPrepared()) result=Script::GetReturnedBool();
	}
	return result;
}

bool Item::EventSkill(Critter* cr, int skill)
{
	bool result=false;
	if(PrepareScriptFunc(ITEM_EVENT_SKILL))
	{
		Script::SetArgObject(this);
		Script::SetArgObject(cr);
		Script::SetArgDword(skill<0?skill:SKILL_OFFSET(skill));
		if(Script::RunPrepared()) result=Script::GetReturnedBool();
	}
	return result;
}

void Item::EventDrop(Critter* cr)
{
	if(!PrepareScriptFunc(ITEM_EVENT_DROP)) return;
	Script::SetArgObject(this);
	Script::SetArgObject(cr);
	Script::RunPrepared();
}

void Item::EventMove(Critter* cr, BYTE from_slot)
{
	if(!PrepareScriptFunc(ITEM_EVENT_MOVE)) return;
	Script::SetArgObject(this);
	Script::SetArgObject(cr);
	Script::SetArgByte(from_slot);
	Script::RunPrepared();
}

void Item::EventWalk(Critter* cr, bool entered, BYTE dir)
{
	if(!PrepareScriptFunc(ITEM_EVENT_WALK)) return;
	Script::SetArgObject(this);
	Script::SetArgObject(cr); // Saved in Act_Move
	Script::SetArgBool(entered);
	Script::SetArgByte(dir);
	Script::RunPrepared();
}
#endif //FONLINE_SERVER

void Item::SetSortValue(ItemPtrVec& items)
{
	WORD sort_value=0x7FFF;
	for(ItemPtrVecIt it=items.begin(),end=items.end();it!=end;++it)
	{
		Item* item=*it;
		if(item==this) continue;
		if(sort_value>=item->GetSortValue()) sort_value=item->GetSortValue()-1;
	}
	Data.SortValue=sort_value;
}

bool SortItemsFunc(const Item& l, const Item& r){return l.Data.SortValue<r.Data.SortValue;}
void Item::SortItems(ItemVec& items){std::sort(items.begin(),items.end(),SortItemsFunc);}

DWORD Item::GetCount()
{
	return IsStackable()?Data.Count:1;
}

void Item::Count_Set(DWORD val)
{
	if(!IsStackable()) return;

	if(Data.Count>val) ItemMngr.SubItemStatistics(GetProtoId(),Data.Count-val);
	else if(Data.Count<val) ItemMngr.AddItemStatistics(GetProtoId(),val-Data.Count);
	Data.Count=val;
}

void Item::Count_Add(DWORD val)
{
	if(!IsStackable()) return;

	Data.Count+=val;
	ItemMngr.AddItemStatistics(GetProtoId(),val);
}

void Item::Count_Sub(DWORD val)
{
	if(!IsStackable()) return;

	if(val>Data.Count) val=Data.Count;
	Data.Count-=val;
	ItemMngr.SubItemStatistics(GetProtoId(),val);
}

#ifdef FONLINE_SERVER
void Item::Repair()
{
	BYTE& flags=Data.TechInfo.BrokenFlags;
	BYTE& broken_count=Data.TechInfo.BrokenCount;
	WORD& deterioration=Data.TechInfo.Deterioration;

	if(FLAG(flags,BI_BROKEN))
	{
		UNSETFLAG(flags,BI_BROKEN);
		deterioration=0;
	}

	STATIC_ASSERT(offsetof(ProtoItem,UserData)==228);
}
#endif

void Item::SetMode(BYTE mode)
{
	if(!IsWeapon())
	{
		mode&=0xF;
		if(mode==USE_USE && !IsCanUse() && !IsCanUseOnSmth()) mode=USE_NONE;
		else if(IsCanUse() || IsCanUseOnSmth()) mode=USE_USE;
		else mode=USE_NONE;
	}
	else
	{
		BYTE use=(mode&0xF);
		BYTE aim=(mode>>4);

		switch(use)
		{
		case USE_PRIMARY: if(Proto->Weapon_ActiveUses&0x1) break; use=0xF; aim=0; break;
		case USE_SECONDARY: if(Proto->Weapon_ActiveUses&0x2) break; use=USE_PRIMARY; aim=0; break;
		case USE_THIRD: if(Proto->Weapon_ActiveUses&0x4) break; use=USE_PRIMARY; aim=0; break;
		case USE_RELOAD: aim=0; if(Proto->Weapon_MaxAmmoCount) break; use=USE_PRIMARY; break;
		case USE_USE: aim=0; if(IsCanUseOnSmth()) break; use=USE_PRIMARY; break;
		default: use=USE_PRIMARY; aim=0; break;
		}

		if(use<MAX_USES && aim && !Proto->Weapon_Aim[use]) aim=0;
		mode=(aim<<4)|(use&0xF);
	}
	Data.Mode=mode;
}

DWORD Item::GetCost1st()
{
	DWORD cost=(Data.Cost?Data.Cost:Proto->Cost);
	//if(IsDeteriorable()) cost-=cost*GetWearProc()/100;
	if(IsWeapon() && Data.TechInfo.AmmoCount)
	{
		ProtoItem* ammo=ItemMngr.GetProtoItem(Data.TechInfo.AmmoPid);
		if(ammo) cost+=ammo->Cost*Data.TechInfo.AmmoCount;
	}
	if(!cost) cost=1;
	return cost;
}

#ifdef FONLINE_SERVER
void Item::SetLexems(const char* lexems)
{
	if(lexems)
	{
		DWORD len=strlen(lexems);
		if(!len)
		{
			SAFEDELA(PLexems);
			return;
		}

		if(len+1>=LEXEMS_SIZE) return;

		if(!PLexems)
		{
			MEMORY_PROCESS(MEMORY_ITEM,LEXEMS_SIZE);
			PLexems=new char[LEXEMS_SIZE];
			if(!PLexems) return;
		}
		memcpy(PLexems,lexems,len+1);
	}
	else
	{
		if(PLexems) MEMORY_PROCESS(MEMORY_ITEM,-LEXEMS_SIZE);
		SAFEDELA(PLexems);
	}
}
#endif

void Item::WeapLoadHolder()
{
	if(!Data.TechInfo.AmmoPid) Data.TechInfo.AmmoPid=Proto->Weapon_DefaultAmmoPid;
	Data.TechInfo.AmmoCount=Proto->Weapon_MaxAmmoCount;
}

#ifdef FONLINE_SERVER
void Item::ContAddItem(Item*& item, DWORD special_id)
{
	if(!IsContainer() || !item) return;

	if(!ChildItems)
	{
		MEMORY_PROCESS(MEMORY_ITEM,sizeof(ItemPtrMap));
		ChildItems=new(nothrow) ItemPtrVec();
		if(!ChildItems) return;
	}

	if(item->IsStackable())
	{
		Item* item_=ContGetItemByPid(item->GetProtoId(),special_id);
		if(item_)
		{
			item_->Count_Add(item->GetCount());
			ItemMngr.ItemToGarbage(item);
			item=item_;
			return;
		}
	}

	item->ACC_CONTAINER.SpecialId=special_id;
	item->SetSortValue(*ChildItems);
	ContSetItem(item);
}

void Item::ContSetItem(Item* item)
{
	if(!ChildItems)
	{
		MEMORY_PROCESS(MEMORY_ITEM,sizeof(ItemPtrMap));
		ChildItems=new(nothrow) ItemPtrVec();
		if(!ChildItems) return;
	}

	if(std::find(ChildItems->begin(),ChildItems->end(),item)!=ChildItems->end())
	{
		WriteLog(__FUNCTION__" - Item already added!\n");
		return;
	}

	ChildItems->push_back(item);
	item->Accessory=ITEM_ACCESSORY_CONTAINER;
	item->ACC_CONTAINER.ContainerId=GetId();
}

void Item::ContEraseItem(Item* item)
{
	if(!IsContainer())
	{
		WriteLog(__FUNCTION__" - Item is not container.\n");
		return;
	}
	if(!ChildItems)
	{
		WriteLog(__FUNCTION__" - Container items null ptr.\n");
		return;
	}
	if(!item)
	{
		WriteLog(__FUNCTION__" - Item null ptr.\n");
		return;
	}

	ItemPtrVecIt it=std::find(ChildItems->begin(),ChildItems->end(),item);
	if(it!=ChildItems->end())
		ChildItems->erase(it);
	else
		WriteLog(__FUNCTION__" - Item not found, id<%u>, pid<%u>, container<%u>.\n",item->GetId(),item->GetProtoId(),GetId());

	item->Accessory=0xd3;

	if(ChildItems->empty()) SAFEDEL(ChildItems);
}

Item* Item::ContGetItem(DWORD item_id, bool skip_hide)
{
	if(!IsContainer() || !ChildItems || !item_id) return NULL;

	for(ItemPtrVecIt it=ChildItems->begin(),end=ChildItems->end();it!=end;++it)
	{
		Item* item=*it;
		if(item->GetId()==item_id)
		{
			if(skip_hide && item->IsHidden()) return NULL;
			SYNC_LOCK(item);
			return item;
		}
	}
	return NULL;
}

void Item::ContGetAllItems(ItemPtrVec& items, bool skip_hide, bool sync_lock)
{
	if(!IsContainer() || !ChildItems) return;

	for(ItemPtrVecIt it=ChildItems->begin(),end=ChildItems->end();it!=end;++it)
	{
		Item* item=*it;
		if(!skip_hide || !item->IsHidden()) items.push_back(item);
	}

#pragma MESSAGE("Recheck after synchronization.")
	if(sync_lock && LogicMT) for(ItemPtrVecIt it=items.begin(),end=items.end();it!=end;++it) SYNC_LOCK(*it);
}

#pragma MESSAGE("Add explicit sync lock.")
Item* Item::ContGetItemByPid(WORD pid, DWORD special_id)
{
	if(!IsContainer() || !ChildItems) return NULL;

	for(ItemPtrVecIt it=ChildItems->begin(),end=ChildItems->end();it!=end;++it)
	{
		Item* item=*it;
		if(item->GetProtoId()==pid && (special_id==-1 || item->ACC_CONTAINER.SpecialId==special_id))
		{
			SYNC_LOCK(item);
			return item;
		}
	}
	return NULL;	
}

void Item::ContGetItems(ItemPtrVec& items, DWORD special_id, bool sync_lock)
{
	if(!IsContainer() || !ChildItems) return;

	for(ItemPtrVecIt it=ChildItems->begin(),end=ChildItems->end();it!=end;++it)
	{
		Item* item=*it;
		if(special_id==-1 || item->ACC_CONTAINER.SpecialId==special_id) items.push_back(item);
	}

#pragma MESSAGE("Recheck after synchronization.")
	if(sync_lock && LogicMT) for(ItemPtrVecIt it=items.begin(),end=items.end();it!=end;++it) SYNC_LOCK(*it);
}

int Item::ContGetFreeVolume(DWORD special_id)
{
	if(!IsContainer()) return 0;

	int cur_volume=0;
	int max_volume=Proto->Container_Volume;
	if(!ChildItems) return max_volume;

	for(ItemPtrVecIt it=ChildItems->begin(),end=ChildItems->end();it!=end;++it)
	{
		Item* item=*it;
		if(special_id==-1 || item->ACC_CONTAINER.SpecialId==special_id) cur_volume+=item->GetVolume();
	}
	return max_volume-cur_volume;
}

bool Item::ContIsItems()
{
	return ChildItems && ChildItems->size();
}

Item* Item::GetChild(DWORD child_index)
{
	if(child_index>=ITEM_MAX_CHILDS || !Proto->ChildPid[child_index]) return NULL;

	if(Accessory==ITEM_ACCESSORY_HEX)
	{
		Map* map=MapMngr.GetMap(ACC_HEX.MapId);
		if(!map) return NULL;
		return map->GetItemChild(ACC_HEX.HexX,ACC_HEX.HexY,Proto,child_index);
	}
	else if(Accessory==ITEM_ACCESSORY_CRITTER)
	{
		Critter* cr=CrMngr.GetCritter(ACC_CRITTER.Id,true);
		if(cr) return cr->GetItemByPid(Proto->ChildPid[child_index]);
	}
	else if(Accessory==ITEM_ACCESSORY_CONTAINER)
	{
		Item* cont=ItemMngr.GetItem(ACC_CONTAINER.ContainerId,true);
		if(cont) return cont->ContGetItemByPid(Proto->ChildPid[child_index],ACC_CONTAINER.SpecialId);
	}
	return NULL;
}
#endif // FONLINE_SERVER

#if defined(FONLINE_CLIENT) || defined(FONLINE_MAPPER)
#include "ResourceManager.h"
#include "SpriteManager.h"

DWORD ProtoItem::GetCurSprId()
{
	AnyFrames* anim=ResMngr.GetItemAnim(PicMap,Dir);
	if(!anim) return 0;

	DWORD beg=0,end=0;
	if(FLAG(Flags,ITEM_SHOW_ANIM)) end=anim->CntFrm-1;
	if(FLAG(Flags,ITEM_SHOW_ANIM_EXT))
	{
		beg=AnimStay[0];
		end=AnimStay[1];
	}

	if(beg>=anim->CntFrm) beg=anim->CntFrm-1;
	if(end>=anim->CntFrm) end=anim->CntFrm-1;
	if(beg>end) std::swap(beg,end);
	DWORD count=end-beg+1;
	DWORD ticks=anim->Ticks/anim->CntFrm*count;
	return anim->Ind[beg+((Timer::GameTick()%ticks)*100/ticks)*count/100];
}

DWORD Item::GetCurSprId()
{
	AnyFrames* anim=ResMngr.GetItemAnim(GetPicMap(),Proto->Dir);
	if(!anim) return 0;

	DWORD beg=0,end=0;
	if(IsShowAnim()) end=anim->CntFrm-1;
	if(IsShowAnimExt())
	{
		beg=Data.AnimStay[0];
		end=Data.AnimStay[1];
	}

	if(beg>=anim->CntFrm) beg=anim->CntFrm-1;
	if(end>=anim->CntFrm) end=anim->CntFrm-1;
	if(beg>end) std::swap(beg,end);
	DWORD count=end-beg+1;
	DWORD ticks=anim->Ticks/anim->CntFrm*count;
	return anim->Ind[beg+((Timer::GameTick()%ticks)*100/ticks)*count/100];
}
#endif