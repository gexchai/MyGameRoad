//
//  BattleMap.h
//  HelloCpp
//
//  Created by ManYou on 13-10-19.
//
//

#ifndef __HelloCpp__BattleMap__
#define __HelloCpp__BattleMap__

#include "cocos2d.h"
#include "BattleTouchListener.h"
#include "Astar.h"
#include "BattleMapTile.h"
#include "BattleMapStatus.h"

USING_NS_CC;

class BattleMap : public Layer
, public BattleTouchListener
, public AstarDelegate
{
public:
    BattleMap();
    ~BattleMap();
    
    virtual bool init();
    CREATE_FUNC(BattleMap);
    
    //# BattleTouchListener
    virtual void BattleTouchEndHappend(float x, float y);
    virtual void BattleTouchBeginHappend(float x, float y);
    
    //# Astar Delegate
    virtual void AstarInitCloseList(Array* arr);

private:
    void initMapInfo();
    
    Point convertToMapPosition(const Point& pos);
    Point convertToWorldPosition(const Point& pos);
    
    BattleMapTile* getMapTile(int x, int y);
    
    bool IsPointInsideOfMap(const Point& pos);
private:
    //map info
    int m_mapXSize;
    int m_mapYSize;
    float m_tileWidth;
    float m_tileHeight;
    Array* m_content;
    
    BattleMapStatus m_status;
};

#endif /* defined(__HelloCpp__BattleMap__) */
