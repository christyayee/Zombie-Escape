#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Actor.h"
#include <string>
#include <list>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    bool isBlocked(Actor* actor, Direction dir);
    bool hasOverlap(Actor* a1, Actor* a2) const;
    bool checkAllOverlaps(Actor* a);
    void addActor(Actor* a);
    bool listContains(Actor* a);
    void updateStatus(int stat);
    int setClosest(Moveable* actor);
    void vomThings(Vomit* vom);

    int getnumCitizens() const;
    bool addVaccine();
    bool addGas();
    bool addLandmine();
    bool useVaccine();
    bool useGas();
    bool useLandmine();

    virtual ~StudentWorld();

private:
    std::list<Actor*> m_actors;
    Penelope* penelope;
    int numCitizens;
    int numVaccines;
    int numGas;
    int numLandmines;
    int status;
};

#endif // STUDENTWORLD_H_
