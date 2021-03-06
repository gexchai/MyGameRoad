//
//  BattleMap.cpp
//  HelloCpp
//
//  Created by ManYou on 13-10-19.
//
//

#include "BattleMap.h"
#include "Global.h"
#include "HeroFactory.h"
#include "BattleMapListener.h"

BattleMap::BattleMap()
: Layer()
, m_mapXSize(0)
, m_mapYSize(0)
, m_tileWidth(0.0f)
, m_tileHeight(0.0f)
, m_content(NULL)
, m_status(kBattleStatus_Undefine)
, m_AstarStart(Point(0, 0))
, m_AstarEnd(Point(0, 0))
, m_currentHero(NULL)
, m_listener(NULL)
{

}
BattleMap::~BattleMap()
{
    if (m_content)
    {
        delete m_content;
        m_content = NULL;
    }
    
    CC_SAFE_RELEASE_NULL(m_content);
}

bool BattleMap::init()
{
    if (!Layer::init())
    {
        return false;
    }
    Size winSize = Director::getInstance()->getWinSize();
    
    auto map = TMXTiledMap::create("defaultLand.tmx");
    map->setPosition(Point((winSize.width-960)/2, 0));
    this->addChild(map, -1);
    
    initMapInfo();
    
    return true;
}

void BattleMap::initMapInfo()
{
    m_mapXSize = 15;
    m_mapYSize = 9;
    m_tileWidth = 64.0f;
    m_tileHeight = 64.0f;
    
    m_content = Array::create();
    m_content->retain();
    for (int i = 0; i < m_mapXSize; ++ i)
    {
        for (int j = 0; j < m_mapYSize; ++ j)
        {
            auto tile = BattleMapTile::create(i, j, kTileContent_Land);
            m_content->addObject(tile);
        }
    }
}

void BattleMap::setStatus(BattleMapStatus status)
{
    m_status = status;
    const char* statusStr = NULL;
    switch (m_status) {
        case kBattleStatus_Undefine:
        {
            statusStr = "kBattleStatus_Undefine";
        }
            break;
        case kBattleStatus_HeroReady:
        {
            statusStr = "kBattleStatus_HeroReady";
        }
            break;
        case kBattleStatus_HeroSelect:
        {
            statusStr = "kBattleStatus_HeroSelect";
        }
            break;
        case kBattleStatus_HeroWalkDone:
        {
            statusStr = "kBattleStatus_HeroWalkDone";
        }
            break;
        case kBattleStatus_HeroWalking:
        {
            statusStr = "kBattleStatus_HeroWalking";
        }
            break;
            
        default:
            break;
    }
    
    CCASSERT(statusStr, "map status error");
    IGLOG("IG_INFO:%s", statusStr);
}

#pragma mark - BattleTouchListener

void BattleMap::BattleTouchEndHappend(float x, float y)
{
    //
    if (m_status == kBattleStatus_HeroWalking)
    {
        return;
    }
    Point mapPos = convertToMapPosition(Point(x, y));
    IGLOG("mapPos (%f, %f)", mapPos.x, mapPos.y);
    if (!IsPointInsideOfMap(mapPos))
    {
        IGLOG("IG_INFO: touch outof map!");
        return;
    }
    Point worldPos = convertToWorldPosition(mapPos);

    BattleMapTile* tile = getMapTile(mapPos.x, mapPos.y);
    if (tile->getContent() == kTileContent_Hero)
    {
        IGLOG("IG_INFO: hero is standing the place");
        auto hero = dynamic_cast<Hero*>(tile->getNode());
        IGLOG("IG_INFO: hero's hp is %d", hero->getHp());
        
        setStatus(kBattleStatus_HeroSelect);
        m_AstarStart = mapPos;
        m_currentHero = hero;
        return;
    }
    
    if (m_status == kBattleStatus_Undefine)
    {
        auto hero = HeroFactory::shareInstance()->createHero("Tank");
        hero->setAnchorPoint(Point(0.5f, 0.0f));
        hero->setPosition(Point(worldPos.x + m_tileWidth / 2, worldPos.y + 10));
        hero->setZOrder(m_mapYSize-mapPos.y);
        this->addChild(hero);
        hero->stayRight();
        tile->setContent(kTileContent_Hero);
        tile->setNode(hero);
    }
    
    if (m_status == kBattleStatus_HeroSelect)
    {
        //处理行走的
        m_AstarEnd = mapPos;
        
        Astar *astar = Astar::create();
        astar->setDelegate(this);
        Array* path = astar->astar();
        for (int i=0; i<path->count(); ++ i)
        {
            AstarNode* node = dynamic_cast<AstarNode*>(path->getObjectAtIndex(i));
            
            IGLOG("(%d, %d)", node->getX(), node->getY());
        }
        HeroMove(path);
        
        setStatus(kBattleStatus_HeroWalking);
    }
    
}

void BattleMap::BattleTouchBeginHappend(float x, float y)
{
    
}

#pragma mark - Astar Delegate

void BattleMap::AstarInitCloseList(Array* arr)
{
    for (int i=0; i < m_mapXSize; ++ i)
    {
        for (int j=0; j < m_mapYSize; ++ j)
        {
            BattleMapTile* tile = getMapTile(i, j);
            BattleMapTileContent content = tile->getContent();
            if (content == kTileContent_Stone || content == kTileContent_Tree
                || content == kTileContent_Hero || content == kTileContent_Undefine)
            {
                AstarNode *node = AstarNode::create();
                node->setX(i);
                node->setY(j);
                arr->addObject(node);
            }
        }
    }
}

AstarNode* BattleMap::AstarStartNode()
{
    AstarNode* node = AstarNode::create();
    node->setX(m_AstarStart.x);
    node->setY(m_AstarStart.y);
    return node;
}
AstarNode* BattleMap::AstarEndNode()
{
    AstarNode* node = AstarNode::create();
    node->setX(m_AstarEnd.x);
    node->setY(m_AstarEnd.y);
    return node;
}

int BattleMap::AstarExpendGInNode(int x, int y)
{
    int expendG = 0;
    BattleMapTile* tile = getMapTile(x, y);
    switch (tile->getContent())
    {
        case kTileContent_Land:
        {
            expendG = 10;
        }
            break;
        case kTileContent_Grass:
        {
            expendG = 15;
        }
            break;
        case kTileContent_Tree:
        {
            expendG = 100;
        }
            break;
        case kTileContent_Hero:
        {
            expendG = 200;
        }
            break;
        case kTileContent_Stone:
        {
            expendG = 300;
        }
            break;
        case kTileContent_Undefine:
        {
            expendG = 0;
        }
            break;
            
        default:
            break;
    }
    return expendG;
}

int BattleMap::AstarExpendHInNode(int x, int y)
{
    static int expend = 10;
    
    int h = (abs(m_AstarEnd.x - x) + abs(m_AstarEnd.y - y)) * expend;
    
    return h;
}

bool BattleMap::AstarIsOutOfMap(int x, int y)
{
    if (x < 0 || y < 0 || x >= m_mapXSize || y >= m_mapYSize) {
        return true;
    }
    return false;
}

#pragma mark - util
Point BattleMap::convertToMapPosition(const Point& pos)
{
    Size winSize = Director::getInstance()->getWinSize();
    float xSpace = (winSize.width - RESOLUTIONSIZE_WIDTH) / 2;
    int x = (int)((pos.x - xSpace)/m_tileWidth);
    int y = (int)(pos.y/m_tileHeight);
    return Point(x, y);
}
Point BattleMap::convertToWorldPosition(const Point& pos)
{
    Size winSize = Director::getInstance()->getWinSize();
    float xSpace = (winSize.width - RESOLUTIONSIZE_WIDTH) / 2;
    float x = pos.x * m_tileWidth + xSpace;
    float y = pos.y * m_tileHeight;
    return Point(x, y);
}
bool BattleMap::IsPointInsideOfMap(const Point& pos)
{
    if (pos.x>=0 && pos.x<m_mapXSize && pos.y>=0 && pos.y<m_mapYSize)
    {
        return true;
    }
    return false;
}

BattleMapTile* BattleMap::getMapTile(int x, int y)
{
    int index = x * m_mapYSize + y;
    IGLOG("m_content->count: %d", m_content->count());
    BattleMapTile* tile = dynamic_cast<BattleMapTile*>(m_content->getObjectAtIndex(index));
    CCASSERT(tile, "tile can't be null");
    CCASSERT(tile->getX() == x && tile->getY() == y, "get error tile");
    return tile;
//    for (int i = 0; i < m_content->count(); ++ i)
//    {
//        BattleMapTile* tile = dynamic_cast<BattleMapTile*>(m_content->getObjectAtIndex(i));
//        if (tile->getX() == x && tile->getY() == y)
//        {
//            return tile;
//        }
//    }
//    
//    return NULL;
}
void BattleMap::setMapTileContent(int x, int y, BattleMapTileContent content, Node* node)
{
    BattleMapTile* tile = getMapTile(x, y);
    CCASSERT(tile, "tile can't null");
    tile->setContent(content);
    tile->setNode(node);
}

void BattleMap::HeroMove(Array* path)
{
    CCASSERT(path->count()>0, "path can't null");
    
    Array* actions = Array::create();
    for (int i = 0; i < path->count()-1; ++ i)
    {
        AstarNode* now = dynamic_cast<AstarNode*>(path->getObjectAtIndex(i));
        AstarNode* next = dynamic_cast<AstarNode*>(path->getObjectAtIndex(i+1));
        
        //before
        CCCallFuncO* before = CCCallFuncO::create(this, callfuncO_selector(BattleMap::HeroMoveBefore), now);
        
        
        Point pos = Point(next->getX() - now->getX(), next->getY() - now->getY());
        CallFunc* call = NULL;
        MoveBy* move = NULL;
        if (pos.x == 0 && pos.y == -1) //down
        {
            call = CallFunc::create(CC_CALLBACK_0(Hero::walkDown, m_currentHero));
            move = MoveBy::create(4.0f/m_currentHero->getWalkSpeed(), Point(0.0f, -m_tileWidth));
        }
        if (pos.x == 0 && pos.y == 1) //up
        {
            call = CallFunc::create(CC_CALLBACK_0(Hero::walkUp, m_currentHero));
            move = MoveBy::create(4.0f/m_currentHero->getWalkSpeed(), Point(0.0f, m_tileWidth));
        }
        if (pos.x == -1 && pos.y == 0) //left
        {
            call = CallFunc::create(CC_CALLBACK_0(Hero::walkLeft, m_currentHero));
            move = MoveBy::create(4.0f/m_currentHero->getWalkSpeed(), Point(-m_tileHeight, 0.0f));
        }
        if (pos.x == 1 && pos.y == 0) //right
        {
            call = CallFunc::create(CC_CALLBACK_0(Hero::walkRight, m_currentHero));
            move = MoveBy::create(4.0f/m_currentHero->getWalkSpeed(), Point(m_tileHeight, 0.0f));
        }
        
        CCASSERT(call && move, "heromove error");
        
        //after
        CCCallFuncO* after = CCCallFuncO::create(this, callfuncO_selector(BattleMap::HeroMoveAfter), next);
        
        actions->addObject(before);
        actions->addObject(call);
        actions->addObject(move);
        actions->addObject(after);
        
    }
    CallFunc* done = CallFunc::create(CC_CALLBACK_0(BattleMap::HeroMoveDone, this));
    actions->addObject(done);
    
    m_currentHero->runAction(Sequence::create(actions));
}

void BattleMap::HeroMoveDone()
{
    
    m_currentHero->walkDone();
    setStatus(kBattleStatus_HeroWalkDone);
    
}
void BattleMap::HeroMoveBefore(Object* start)
{
    //将起点的tile重置为land
    AstarNode* node = dynamic_cast<AstarNode*>(start);
    CCASSERT(node, "node can't null");
    setMapTileContent(node->getX(), node->getY(), kTileContent_Land);
}
void BattleMap::HeroMoveAfter(Object* end)
{
    //将终点的tile设置为hero,并将当前的hero设置到tile的node上
    AstarNode* node = dynamic_cast<AstarNode*>(end);
    CCASSERT(node, "node can't null");
    setMapTileContent(node->getX(), node->getY(), kTileContent_Hero, m_currentHero);
    
    //根据地图坐标修改当前英雄的 zOrder
    m_currentHero->setZOrder(m_mapYSize - node->getY());
}