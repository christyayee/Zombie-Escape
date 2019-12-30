#include "StudentWorld.h"
#include "GameConstants.h"
#include "Level.h"
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
}

int StudentWorld::init()//need multiple levels
{
    numCitizens = 0;
    numVaccines = 0;
    numGas = 0;
    numLandmines = 0;
    status = GWSTATUS_CONTINUE_GAME;
    
    Level lev(assetPath());
    //string levelFile = "level04.txt";
    ostringstream nextLevel;
    nextLevel.fill('0');
    nextLevel << "level" << setw(2) << getLevel() << ".txt";
    string levelFile  = nextLevel.str();
    Level::LoadResult result = lev.loadLevel(levelFile);
    if (result == Level::load_fail_file_not_found)
        return GWSTATUS_PLAYER_WON;
    else if (result == Level::load_fail_bad_format)
        return GWSTATUS_LEVEL_ERROR;
    else if (result == Level::load_success)
    {
        for (int i = 0; i < LEVEL_WIDTH; i++)
        {
            for (int j = 0; j < LEVEL_HEIGHT; j++)
            {
                Level::MazeEntry element = lev.getContentsOf(i,j);
                switch(element)
                {
                    case Level::player:
                        penelope = new Penelope(IID_PLAYER,SPRITE_WIDTH * i, SPRITE_HEIGHT * j, 0, 0, 1, this);
                        break;
                    case Level::wall:
                        m_actors.push_back(new Wall(IID_WALL,SPRITE_WIDTH * i,SPRITE_HEIGHT * j, 0, 0, 1, this));
                        break;
                    case Level::exit:
                        m_actors.push_back(new Exit(IID_EXIT,SPRITE_WIDTH * i,SPRITE_HEIGHT * j, 0, 1, 1, this));
                        break;
                    case Level::vaccine_goodie:
                        m_actors.push_back(new GVaccine(IID_VACCINE_GOODIE,SPRITE_WIDTH * i,SPRITE_HEIGHT * j, 0, 1, 1, this));
                        break;
                    case Level::gas_can_goodie:
                        m_actors.push_back(new GGas(IID_GAS_CAN_GOODIE,SPRITE_WIDTH * i,SPRITE_HEIGHT * j, 0, 1, 1, this));
                        break;
                    case Level::landmine_goodie:
                        m_actors.push_back(new GLandmine(IID_LANDMINE_GOODIE,SPRITE_WIDTH * i,SPRITE_HEIGHT * j, 0, 1, 1, this));
                        break;
                    case Level::pit:
                        m_actors.push_back(new Pit(IID_PIT,SPRITE_WIDTH * i,SPRITE_HEIGHT * j, 0, 0, 1, this));
                        break;
                    case Level::citizen:
                        m_actors.push_back(new Citizen(IID_CITIZEN,SPRITE_WIDTH * i,SPRITE_HEIGHT * j, 0, 0, 1, this));
                        numCitizens++;
                        break;
                    case Level::dumb_zombie:
                        m_actors.push_back(new DumbZombie(IID_ZOMBIE,SPRITE_WIDTH * i, SPRITE_HEIGHT * j, 0, 0, 1, this));
                        break;
                    case Level::smart_zombie:
                        m_actors.push_back(new SmartZombie(IID_ZOMBIE,SPRITE_WIDTH * i, SPRITE_HEIGHT * j, 0, 0, 1, this));
                        break;
                    case Level::empty:
                        break;
                }
            }
        }
    }
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    //Score: 004500  Level: 27  Lives: 3  Vaccines: 2  Flames: 16  Mines: 1  Infected: 0
    ostringstream oss;
    oss.setf(ios::fixed);
    oss.precision(2);
    oss.fill('0');
    
    if(getScore() >= 0)
        oss << "Score: " << setw(6) << getScore();
    else
        oss << "Score: -" << setw(5) << abs(getScore());
    oss.fill(' ');
    oss << "  Level: " << setw(2) << getLevel();
    oss << "  Lives: " << setw(1) << getLives();
    oss << "  Vaccines: " << setw(2) << numVaccines;
    oss << "  Flames: " << setw(2) << numGas;
    oss << "  Mines: " << setw(2) << numLandmines;
    oss << "  Infected: " << setw(1) << penelope -> getInfectTime();
    string convert = oss.str();
    setGameStatText(convert);
    
    int val;
    getKey(val);
    penelope->setKey(val);
    penelope->doSomething();
    list<Actor*> :: iterator it = m_actors.begin();
    while (it != m_actors.end())
    {
        (*it)->doSomething();
        (*it)->setInteractor(nullptr);
        it++;
    }
    
    it = m_actors.begin();
    while (it != m_actors.end())
    {
        if (!(*it)->isAlive())
        {
            (*it)->setInteractor(nullptr);
            if ((*it)->isInfectable()) //if dead Citizen
                numCitizens--;
            delete *it;
            it = m_actors.erase(it);
        }
        else
            it++;
    }
    if (status == GWSTATUS_PLAYER_DIED)
        decLives();
    return status;
}

void StudentWorld::cleanUp()
{
    if (penelope != nullptr)
    {
        delete penelope;
        list<Actor*> :: iterator it = m_actors.begin();
        while (it != m_actors.end())
        {
            delete *it;
            it = m_actors.erase(it);
        }
        penelope = nullptr;
    }
}

int StudentWorld::getnumCitizens() const
{
    return numCitizens;
}

//returns true if actor is blocked in direction of dir, returns false otherwise
//if actor is a Zombie and *it is a Human, sets Zombie's interactor to Human that it's blocked by
bool StudentWorld::isBlocked(Actor* actor, Direction dir)
{
    list<Actor*> :: iterator it = m_actors.begin();
    m_actors.push_back(penelope);
    while (it != m_actors.end())
    {
        if ((*it) != actor && (*it)->canBlock())
        {
            double x = actor->getX() - (*it)->getX();
            double y = actor->getY() - (*it)->getY();
            switch (dir)
            {
                case 180:
                    if ((*it)->getX() <= actor->getX())
                    {
                        if (abs(x) <= SPRITE_WIDTH && abs(y) <= SPRITE_HEIGHT-1)
                        {
                            if ( actor->canInfect() && (*it)->isInfectable())
                                actor->setInteractor((*it));
                            m_actors.pop_back();
                            return true;
                        }
                    }
                    break;
                case 0:
                    if ((*it)->getX() >= actor->getX())
                    {
                        if (abs(x) <= SPRITE_WIDTH && abs(y) <= SPRITE_HEIGHT-1)
                        {
                            if ( actor->canInfect() && (*it)->isInfectable())
                                actor->setInteractor((*it));
                            m_actors.pop_back();
                            return true;
                        }
                    }
                    break;
                case 90:
                    if ((*it)->getY() >= actor->getY())
                    {
                        if (abs(x) <= SPRITE_WIDTH-1 && abs(y) <= SPRITE_HEIGHT)
                        {
                            if ( actor->canInfect() && (*it)->isInfectable())
                                actor->setInteractor((*it));
                            m_actors.pop_back();
                            return true;
                        }
                    }
                    break;
                case 270:
                    if ((*it)->getY() <= actor->getY())
                    {
                        if (abs(x) <= SPRITE_WIDTH-1 && abs(y) <= SPRITE_HEIGHT)
                        {
                            if ( actor->canInfect() && (*it)->isInfectable())
                                actor->setInteractor((*it));
                            m_actors.pop_back();
                            return true;
                        }
                    }
                    break;
            }
        }
        it++;
    }
    m_actors.pop_back();
    return false;
}

//returns true if Actor* a1 and Actor* a2 are overlapping
bool StudentWorld::hasOverlap(Actor* a1, Actor* a2) const
{
    double x = abs(a1->getX() - a2->getX());
    double y = abs(a1->getY() - a2->getY());
    double distance  = (x*x) + (y*y);
    if (distance <= 100)
        return true;
    return false;
}

//checks overlaps with Actor* a and every other actor in m_actors and penelope
//only calls react() if there is an overlapping interactor
bool StudentWorld::checkAllOverlaps(Actor* a)
{
    //default assumes not interacting/overlapping with anything
    a->setInteractor(nullptr);
    bool status = false;
    
    //check overlaps within m_actors list
    list<Actor*> :: iterator it = m_actors.begin();
    while (it != m_actors.end())
    {
        if (a != (*it) && a->isAlive() && (*it)->isAlive() && hasOverlap(a, (*it)))
        {
            a->setInteractor(*it);
            if (a->react())
                status = true;
        }
        it++;
    }
    
    //check overlap with Penelope
    if (a->isAlive() && (penelope)->isAlive() && hasOverlap(a, penelope))
    {
        a->setInteractor(penelope);
        if (a->react())
            status = true;
    }
    return status;
}

//returns distance of closest wanted actor(Zombie/Citizen)
//sets mover's closest pointer to closest wanted actor
//sets mover's interactor pointer to penelope
int StudentWorld::setClosest(Moveable* mover)
{
    mover->setClosest(nullptr);
    mover->setInteractor(nullptr);
    list<Actor*> :: iterator it;
    double bestD = INT_MAX;
    
    for (it = m_actors.begin(); it != m_actors.end(); it++)
    {
        if ( (mover->isInfectable() && (*it)->canInfect())
            || (mover->canInfect() && (*it)->isInfectable()) ) //Citizen looking for closest Zombie, or Zombie looking for Citizen
        {
            double x = abs(mover->getX() - (*it)->getX());
            double y = abs(mover->getY() - (*it)->getY());
            double xy = (x*x) + (y*y);
            if(xy <= bestD)
            {
                bestD = xy;
                mover->setClosest(*it);
            }
        }
    }
    
    //set Penelope to interactor if close enough
    double px = abs(mover->getX() - penelope->getX());
    double py = abs(mover->getY() - penelope->getY());
    double pxy = (px*px) + (py*py);
    if (pxy <= 6400)
        mover->setInteractor(penelope);
    
    return bestD;
}

//calls react on all Infectables with vom as its interactor
void StudentWorld::vomThings(Vomit* vom)
{
    list<Actor*> :: iterator it = m_actors.begin();
    while (it != m_actors.end())
    {
        if ( (*it)->isInfectable() && (*it)->getInteractor() == vom ) //if Human's interactor is vom
            (*it)->react();
        it++;
    }
    if (penelope->getInteractor() == vom)
        penelope->react();
}


//returns true if actor is in m_actors, returns false otherwise
bool StudentWorld::listContains(Actor* a)
{
    return (a != penelope);
//    list<Actor*> :: iterator it = m_actors.begin();
//    while (it != m_actors.end())
//    {
//        if (*it == a)
//            return true;
//        it++;
//    }
//    return false;
}

void StudentWorld::updateStatus(int stat)
{
    status = stat;
}

void StudentWorld::addActor(Actor* a)
{
    m_actors.push_back(a);
}

bool StudentWorld::addVaccine()
{
    numVaccines++;
    return true;
}
bool StudentWorld::addGas()
{
    numGas = numGas + 5;
    return true;
}
bool StudentWorld::addLandmine()
{
    numLandmines = numLandmines + 2;
    return true;
}
bool StudentWorld::useVaccine()
{
    if (numVaccines > 0)
    {
        numVaccines--;
        return true;
    }
    return false;
}
bool StudentWorld::useGas()
{
    if (numGas > 0)
    {
        numGas--;
        return true;
    }
    return false;
}
bool StudentWorld::useLandmine()
{
    if (numLandmines > 0)
    {
        numLandmines--;
        return true;
    }
    return false;
}

StudentWorld::~StudentWorld()
{
    cleanUp();
}
