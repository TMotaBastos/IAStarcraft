#include "AAExample.h"
#include <iostream>
#include <Windows.h>
#include <cmath>

using namespace BWAPI;
using namespace Filter;
using namespace std;

static int ZEALOT_RUSH = 0;
static int DRAGOON_MODE = 1;

static int SCOUT = 10;
static int BUILDING = 11;
static int COLLECTING_MINERAL = 12;
static int COLLECTING_GAS = 13;
static int IDLE = 14;

static int ATTACK = 20;

int mode;

// AA: Do a blackboard, don't use it directly! (or at least don't use it static, use a singleton)
static bool GameOver;
HANDLE ghMutex;
int workers;
int gateways;
int unit_zealot;
vector<UnitType> ger_producao;
set< pair<int, UnitType> > lista_producao;
int probe;
int zealot;
int dragoon;
int gateway;
int pylon;
map<int, int> trabalhadores;
set< pair< int, pair<TilePosition, UnitType> > > workers_commands;
int coletandoGas;
int scouteando;
int construindo;
vector< pair<TilePosition, UnitType> > build_list;

set< pair<int, TilePosition> > militar_commands;

vector<TilePosition> posToVisit;

int lastChecked;
bool vouConstruir;
bool vouAtacar;

Unit ref_base;
vector<Unit> ref_gateways;
Unit ref_pylon;

Unit ref_base_inimiga;
TilePosition pos_base_inimiga;

void AAExample::onStart()
{
	GameOver = false;
	workers = 0;
	gateways = 0;
	probe = 4;
	zealot = 0;
	dragoon = 0;
	gateway = 0;
	pylon = 0;
	unit_zealot = 0;
	mode = ZEALOT_RUSH;
	lastChecked = 0;
	vouConstruir = false;
	coletandoGas = 0;
	scouteando = 0;
	construindo = 0;
	vouAtacar = false;

	ref_base_inimiga = NULL;

	// Create a mutex with no initial owner
	ghMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	// Hello World!
	//Broodwar->sendText("My badass agent is ready for battle!");

	// Set the command optimization level so that common commands can be grouped
	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(2);

	//construcao
	CreateThread(NULL, 0, Produtor, (LPVOID)NULL, 0, NULL);
	CreateThread(NULL, 0, Gerenciador_Producao, (LPVOID)NULL, 0, NULL);
	CreateThread(NULL, 0, Gerenciador_Trabalhador, (LPVOID)NULL, 0, NULL);
	CreateThread(NULL, 0, Construcao, (LPVOID)NULL, 0, NULL);
	CreateThread(NULL, 0, Estrategico, (LPVOID)NULL, 0, NULL);
	CreateThread(NULL, 0, Militar, (LPVOID)NULL, 0, NULL);

	TilePosition tp = TilePositions::Origin;
	int alt = Broodwar->mapHeight() / 3;
	int larg = Broodwar->mapWidth() / 3;

	for (int i = 1; i <= 3; i++)
	{
		for (int j = 1; j <= 3; j++)
		{
			tp.x = (alt*(i-1))+(alt/2)*i;
			tp.y = (larg*(i-1))+(larg/2)*j;
			posToVisit.push_back(tp);
		}
	}

	// Retrieve you and your enemy's races. enemy() will just return the first enemy. AA: Not a problem with just 2.
	// If you wish to deal with multiple enemies then you must use enemies().
	//if (Broodwar->enemy()) // First make sure there is an enemy AA: Make sure to check EVERYTHING.
	//	Broodwar << "The matchup is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;

}

// For the hardcore ones, you can use Thread Pool closing it at onEnd.
void AAExample::onEnd(bool isWinner)
{
	// Called when the game ends
	GameOver = true;
	if (isWinner)
	{
		// AA: Do whatever you want, but make sure to not break anything.
		// Log your win here!
	}
	// you don't need it if you end your threads without GameOver.
	Sleep(100); // Enough time to end the threads.
}

void AAExample::onFrame()
{
	// Called once every game frame

	// Display the game frame rate as text in the upper left area of the screen
	Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
	Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());
	//Broodwar << "Minerios = " << Broodwar->getStaticMinerals().size() << std::endl;
}

void AAExample::onSendText(std::string text)
{
	// Send the text to the game if it is not being processed.
	//Broodwar->sendText("%s", text.c_str());
	// Make sure to use %s and pass the text as a parameter,
	// otherwise you may run into problems when you use the %(percent) character!
}

void AAExample::onReceiveText(BWAPI::Player player, std::string text)
{ // AA: You won't need it because we don't have a team game, but it can change.
	// Parse the received text
	//Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void AAExample::onPlayerLeft(BWAPI::Player player)
{
	// Interact verbally with the other players in the game by
	// announcing that the other player has left.
	//Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void AAExample::onNukeDetect(BWAPI::Position target)
{
	// AA: If you can make a nuke in 10 minutes, good for you.
	// Check if the target is a valid position
	if (target)
	{
		// if so, print the location of the nuclear strike target
		//Broodwar << "Nuclear Launch Detected at " << target << std::endl;
	}
	else
	{
		// Otherwise, ask other players where the nuke is!
		//Broodwar->sendText("Where's the nuke?");
	}
	// You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
}

void AAExample::onUnitDiscover(BWAPI::Unit unit)
{
}

void AAExample::onUnitEvade(BWAPI::Unit unit)
{
}

void AAExample::onUnitShow(BWAPI::Unit unit)
{
}

void AAExample::onUnitHide(BWAPI::Unit unit)
{
}

void AAExample::onUnitCreate(BWAPI::Unit unitOn)
{
	// For each created unit...
	if (unitOn->getType().isWorker()) // or  == BWAPI::UnitTypes::Terran_SCV if terran
	{
		CreateThread(NULL, 0, thisShouldBeAClassButImTooLazyToDoIt_Worker, (LPVOID)unitOn, 0, NULL);
		//workers++;
		//Broodwar->sendText("Probe = %d", probe);
		//probe--;
		//Broodwar->sendText("Trabalhadores: %d , probe = %d", workers, probe);
	}
	// You can do a direct comparison like  == BWAPI::UnitTypes::Terran_Command_Center too.
	else if (unitOn->getType().isResourceDepot())
	{
		ref_base = unitOn;
		CreateThread(NULL, 0, GeneralOrManagerOrGerenteOrSomethingLikeThat, (LPVOID)unitOn, 0, NULL);
	}
	else if (unitOn->getType() == UnitTypes::Protoss_Pylon)
	{
		vouConstruir = false;
		ref_pylon = unitOn;
		//pylon--;
		CreateThread(NULL, 0, Building_Pylon, (LPVOID)unitOn, 0, NULL);
		//Broodwar->sendText("Pylon criado");
	}
	else if (unitOn->getType() == UnitTypes::Protoss_Gateway)
	{
		vouConstruir = false;
		ref_gateways.push_back(unitOn);
		gateways++;
		//gateway--;
		CreateThread(NULL, 0, Building_Gateway, (LPVOID)unitOn, 0, NULL);
		//Broodwar->sendText("Gateways: %d", gateways);
	}
	else if (unitOn->getType() == UnitTypes::Protoss_Zealot)
	{
		//unit_zealot++;
		zealot--;
		CreateThread(NULL, 0, Zealot, (LPVOID)unitOn, 0, NULL);
	}
	else if (unitOn->getType() == UnitTypes::Protoss_Dragoon)
		dragoon--;
}

void AAExample::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType().isWorker())
	{
		map<int, int>::iterator it = trabalhadores.find(unit->getID());
		if (it != trabalhadores.end())
			trabalhadores.erase(it);
		//Broodwar->sendText("Morreu!");
	}
	else if (unit->getType() == UnitTypes::Protoss_Gateway)
	{
		//for (int i = 0; i < ref_gateways.size(); i++)
		for (std::vector<Unit>::const_iterator i = ref_gateways.begin(); i != ref_gateways.end(); i++)
		{
			if ((*i)->getID() == unit->getID())
			{
				ref_gateways.erase(i);
				gateways--;
				break;
			}
		}
	}
}

void AAExample::onUnitMorph(BWAPI::Unit unit)
{
	// AA: This is a important function for the Zerg player.
}

void AAExample::onUnitRenegade(BWAPI::Unit unit)
{
}

void AAExample::onSaveGame(std::string gameName)
{
}

void AAExample::onUnitComplete(BWAPI::Unit unit)
{
}

DWORD WINAPI AAExample::Estrategico(LPVOID param)
{

	DWORD dwWaitResult;
	bool jaelvis = false;

	while (true){

		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		if (GameOver) {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		}
		// Some things are commom between units, so you can apply a little of OO here.
		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED)
		{
			if (workers == 6 && scouteando == 0 && !jaelvis)
			{
				/*TilePosition tp = ref_base->getTilePosition();
				if (tp.x < 10)
				{
					tp.x = 60;
					tp.y = 60;
				}
				else
				{
					tp.x = 10;
					tp.y = 10;
				}

				Broodwar << "Height = " << Broodwar->mapHeight() << " - Width = " << Broodwar->mapWidth() << std::endl;*/
				for (int i = 0; i < 9; i++)
				{
					militar_commands.insert(make_pair(SCOUT, posToVisit[i]));
				}
				//militar_commands.insert(make_pair(SCOUT, tp));
				jaelvis = true;
			}
			if (ref_base_inimiga != NULL && unit_zealot >= 2 && !vouAtacar)
			{
				//TilePosition topzera = ref_base_inimiga->getTilePosition();
				//Position pos = Position(pos_base_inimiga);
				//int kk = 0;
				militar_commands.insert(make_pair(ATTACK, pos_base_inimiga));
			}

			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}
		}
		Sleep(10);
	}
}

DWORD WINAPI AAExample::Militar(LPVOID param)
{

	DWORD dwWaitResult;
	int cont = 1;

	while (true){

		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		if (GameOver) {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		}
		// Some things are commom between units, so you can apply a little of OO here.
		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED)
		{
			for (set< pair<int, TilePosition> >::const_iterator it = militar_commands.begin(); it != militar_commands.end();)
			{
				if ((*it).first == SCOUT)
				{
					/*if (unit_zealot > 0)
					{

					}
					else
					{*/
						//Broodwar << "Militar = " << cont << std::endl;
						cont++;
						workers_commands.insert(make_pair(SCOUT, make_pair((*it).second, UnitTypes::None)));
						it = militar_commands.erase(it);
						break;
					//}
					
				}
				else if ((*it).first == ATTACK)
				{
					for (std::list<Unit>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++)
					{
						//if ((*i)->getType() == (*it).second.whatBuilds().first)
						if ((*i)->getType() == UnitTypes::Protoss_Zealot || (*i)->getType() == UnitTypes::Protoss_Dragoon)
						{
							Position pos = /*Position(((*it).second.x - 10), ((*it).second.y - 10));*/(Position) (*it).second;
							pos.x -= 20;
							pos.y -= 20;
							
							(*i)->move(pos/*(Position) (*it).second*/);
							Error lastErr = Broodwar->getLastError();
							if (lastErr == Errors::None)
							{
								//it = militar_commands.erase(it);
								//break;
							}

							//}
							//break;
						}
					}
					vouAtacar = true;
					it = militar_commands.erase(it);
					break;
				}
				//02/07
				++it;
				
			}


			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}
		}
		Sleep(10);
	}
}

DWORD WINAPI AAExample::Produtor(LPVOID param)
{

	DWORD dwWaitResult;
	bool entrou = false;

	while (true){

		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		if (GameOver) {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		}
		// Some things are commom between units, so you can apply a little of OO here.
		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED)
		{
			//for (auto it : lista_producao)
			int minerioAtual = Broodwar->self()->minerals();
			int gasAtual = Broodwar->self()->gas();
			
			for (set< pair<int, UnitType> >::const_iterator it = lista_producao.begin(); it != lista_producao.end(); )
			{
				if ((*it).second.mineralPrice() <= minerioAtual/*Broodwar->self()->minerals()*/ &&
					(*it).second.gasPrice() <= gasAtual/*Broodwar->self()->gas()*/)
				{
					/*if ((*it).second.supplyRequired() > (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed()))
					{
						++it;
						//continue;
						break;
					}*/

					if ((*it).second == UnitTypes::Protoss_Probe && !vouConstruir)
					{
						if (ref_base->isIdle())
						{
							entrou = true;
							//Broodwar->sendText("Probe - Minerio = %d", minerioAtual);
							minerioAtual -= (*it).second.mineralPrice();
							gasAtual -= (*it).second.gasPrice();
							ref_base->train((*it).second);
							////++it;
							//it = lista_producao.erase(it);
							//break;
							Error lastErr = Broodwar->getLastError();
							if (lastErr == Errors::None)
							{
								it = lista_producao.erase(it);
							}
							break;
						}
						if (!entrou)
						{
							++it;
						}
						entrou = false;
					}
					else if (((*it).second == UnitTypes::Protoss_Zealot || (*it).second == UnitTypes::Protoss_Dragoon) && !vouConstruir)
					{
						for (int i = 0; i < ref_gateways.size(); i++)
						{
							if (ref_gateways[i]->isIdle())
							{
								entrou = true;
								minerioAtual -= (*it).second.mineralPrice();
								gasAtual -= (*it).second.gasPrice();
								ref_gateways[i]->train((*it).second);
								////++it;
								//it = lista_producao.erase(it);
								Error lastErr = Broodwar->getLastError();
								if (lastErr == Errors::None)
								{
									it = lista_producao.erase(it);
									break;
								}
								else
								{
									entrou = false;
								}
							}
						}
						if (!entrou)
						{
							++it;
						}
						entrou = false;
						//02/07
						//break;
					}
					//else
					else if ((*it).second == UnitTypes::Protoss_Assimilator || (*it).second == UnitTypes::Protoss_Gateway || (*it).second == UnitTypes::Protoss_Pylon || (*it).second == UnitTypes::Protoss_Cybernetics_Core)
					{
						/*//Broodwar->sendText("Tamanho da lista = %d", lista_producao.size());
						for (std::list<Unit>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++)
						{
							//if ((*i)->getType() == (*it).second.whatBuilds().first)
							if ((*i)->getType().isWorker() == true)
							{
								entrou = true;
								//Broodwar->sendText("Pylon - Minerio = %d", minerioAtual);
								minerioAtual -= (*it).second.mineralPrice();
								gasAtual -= (*it).second.gasPrice();
								//Broodwar->sendText("vai construir o pylon!");
								TilePosition targetBuildLocation = Broodwar->getBuildLocation((*it).second, (*i)->getTilePosition());
								//if (targetBuildLocation)
								//{
									*//*if (*//*(*i)->build((*it).second, targetBuildLocation);//)
								Error lastErr = Broodwar->getLastError();
								if (lastErr == Errors::None)
								{
									vouConstruir = true;
									it = lista_producao.erase(it);
									break;
								}

								//}
								//break;
							}
						}
						if (!entrou)
						{
							++it;
						}
						entrou = false;*/
						vouConstruir = true;
						//02/07
						//Broodwar->sendText("...");
						////02/07
						TilePosition targetBuildLocation;
						//if (ref_pylon) targetBuildLocation = Broodwar->getBuildLocation((*it).second, ref_pylon->getTilePosition());
						//02/07
						///*else*/ targetBuildLocation = Broodwar->getBuildLocation((*it).second, ref_base->getTilePosition());
						//Broodwar << ref_base->getTilePosition() << std::endl;
						//Broodwar->
						//02/07
						/*
						if (ref_base->getTilePosition().x < 10)
						{
							targetBuildLocation.x++;
							targetBuildLocation.y++;
						}
						else
						{
							targetBuildLocation.x -= 2;
							targetBuildLocation.y -= 2;
						}
						*/
						workers_commands.insert(make_pair(BUILDING, make_pair(targetBuildLocation, (*it).second)));
						it = lista_producao.erase(it);
						break;
					}
				}
				else
				{
					if ((*it).second == UnitTypes::Protoss_Zealot && unit_zealot % 2 == 1)
					{
						++it;
						continue;
					}
					break;
				}

			}

			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}
		}
		Sleep(10);
	}
}

DWORD WINAPI AAExample::Gerenciador_Trabalhador(LPVOID param)
{

	DWORD dwWaitResult;
	int cont = 1;

	while (true){

		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		if (GameOver) {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		}
		// Some things are commom between units, so you can apply a little of OO here.
		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED)
		{

			for (set< pair<int, pair<TilePosition, UnitType> > >::const_iterator it = workers_commands.begin(); it != workers_commands.end();)
			{
				if ((*it).first == SCOUT)
				{
					//Broodwar << "Ger Trabalhador = " << cont << std::endl;
					if (scouteando == 0)
					{
						for (std::list<Unit>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++)
						{
							//if ((*i)->getType() == (*it).second.whatBuilds().first)
							if ((*i)->getType().isWorker() == true && ((*i)->isIdle() || (*i)->isGatheringMinerals()))
							{
								(*i)->move((Position)(*it).second.first);
								Error lastErr = Broodwar->getLastError();
								if (lastErr == Errors::None)
								{
									it = workers_commands.erase(it);
									cont++;
									trabalhadores[(*i)->getID()] = SCOUT;
									scouteando++;
									break;
								}

								//}
								//break;
							}
						}
					}
					else
					{
						for (std::list<Unit>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++)
						{
							//if ((*i)->getType() == (*it).second.whatBuilds().first)
							if ((*i)->getType().isWorker() == true)
							{
								int topzera = (*i)->getID();
								map<int, int>::const_iterator tf = trabalhadores.find((*i)->getID());
								if (tf != trabalhadores.end()) 
									if (tf->second != SCOUT) continue;
								//Broodwar->sendText("topster demais pai!");
								(*i)->move((Position)(*it).second.first, true);
								Error lastErr = Broodwar->getLastError();
								if (lastErr == Errors::None)
								{
									cont++;
									it = workers_commands.erase(it);
									//trabalhadores[(*i)->getID()] = SCOUT;
									//scouteando++;
									break;
								}

								//}
								//break;
							}
						}
					}
				}
				else if ((*it).first == BUILDING)
				{
					/*build_list.push_back((*it).second);
					if (construindo == 0)
					{
						Position posBuilding = (Position) (*it).second.first;
						Position posWorker;
						Unit closerWorker;
						double minDistance = 999999999;
						double distance;
						for (std::list<Unit>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++)
						{
							if ((*i)->getType().isWorker() == true && ((*i)->isGatheringMinerals() || (*i)->isIdle()))
							{
								posWorker = (*i)->getPosition();
								distance = sqrt(pow((posBuilding.x - posWorker.x), 2) + pow((posBuilding.y - posWorker.y), 2));
								if (distance < minDistance)
								{
									Broodwar->sendText("Achou um trabalhador topzera");
									minDistance = distance;
									closerWorker = (*i);
								}
							}
						}
						construindo++;
						//build_list.push_back(make_pair(Broodwar->getBuildLocation((*it).second.second, closerWorker->getTilePosition()), (*it).second.second));
						trabalhadores[closerWorker->getID()] = BUILDING;
					}
					it = workers_commands.erase(it);*/
					//Broodwar->sendText("Tamanho da lista = %d", lista_producao.size());
					for (std::list<Unit>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++)
					{
						//if ((*i)->getType() == (*it).second.whatBuilds().first)
						if ((*i)->getType().isWorker() == true)
						{
							map<int, int>::const_iterator tf = trabalhadores.find((*i)->getID());
							if (tf != trabalhadores.end())
								if (tf->second == SCOUT) continue;
							//Broodwar->sendText("Pylon - Minerio = %d", minerioAtual);
							//Broodwar->sendText("vai construir o pylon!");
							TilePosition targetBuildLocation = Broodwar->getBuildLocation((*it).second.second, (*i)->getTilePosition());
							//if (targetBuildLocation)
							//{
							/*if (*/(*i)->build((*it).second.second, targetBuildLocation);//)
							Error lastErr = Broodwar->getLastError();
							if (lastErr == Errors::None)
							{
								it = workers_commands.erase(it);
								break;
							}

						//}
						//break;
						}
					}
				}
			}

			for (std::list<Unit>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++)
			{
				if ((*i)->getType().isWorker() == true)
				{
					if (mode == DRAGOON_MODE && coletandoGas < 2)
					{
						//coletar gas
						coletandoGas++;
						trabalhadores[(*i)->getID()] = COLLECTING_GAS;
					}
					else if ((*i)->isIdle() == true)
					{
						//coletar minerio
						map<int, int>::const_iterator tf = trabalhadores.find((*i)->getID());
						if (tf != trabalhadores.end())
							if (tf->second == SCOUT) scouteando--;
						trabalhadores[(*i)->getID()] = COLLECTING_MINERAL;
					}
				}
			}

			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}
		}
		Sleep(10);
	}
}

DWORD WINAPI AAExample::Gerenciador_Producao(LPVOID param)
{

	DWORD dwWaitResult;

	//Broodwar->sendText("Comecou gerenciador de producao!");
	//Broodwar->sendText("workers = %d , probe = %d", workers, probe);

	while (true){

		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		if (GameOver) {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		}
		// Some things are commom between units, so you can apply a little of OO here.
		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED)
		{
			while (!ger_producao.empty())
			{
				UnitType it = ger_producao.back();
				ger_producao.pop_back();

				int prioridade = 0;
				if (it == UnitTypes::Protoss_Pylon)
				{
					//Broodwar->sendText("mandou um pylon para lista de producao!");
					prioridade = 1;
				}
				else if (it == UnitTypes::Protoss_Gateway)
				{
					//Broodwar->sendText("mandou um gateway para lista de producao!");
					prioridade = 2;
				}
				else if (it == UnitTypes::Protoss_Cybernetics_Core)
					prioridade = 3;

				lista_producao.insert(make_pair(prioridade, it));
				

				
			}

			//Broodwar->sendText("workers = %d , probe = %d", workers, probe);

			if ((workers < 6 && probe == 0)/* || probe == 0*/)
			{
				//Broodwar->sendText("Constroi um brodinho!");
				lista_producao.insert(make_pair(6, UnitTypes::Protoss_Probe));
				probe++;
			}
			else if (mode == ZEALOT_RUSH && zealot < gateways)
			{
				//Broodwar->sendText("workers = %d , probe = %d , tam = %d", workers, probe, lista_producao.size());
				lista_producao.insert(make_pair(4, UnitTypes::Protoss_Zealot));
				zealot++;
			}
			else if (mode == DRAGOON_MODE)
			{
				if (dragoon > 0)
				{
					lista_producao.insert(make_pair(4, UnitTypes::Protoss_Zealot));
					zealot++;
				}
				else
				{
					lista_producao.insert(make_pair(5, UnitTypes::Protoss_Dragoon));
					dragoon++;
				}
			}

			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}
		}
		Sleep(10);
	}
}

DWORD WINAPI AAExample::Construcao(LPVOID param)
{

	DWORD dwWaitResult;
	bool primeira = true;

	while (true){

		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		if (GameOver) {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		}
		// Some things are commom between units, so you can apply a little of OO here.
		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED)
		{
			if ((Broodwar->self()->supplyUsed() >= (Broodwar->self()->supplyTotal() - 2) || (workers == 6 && primeira)) && pylon == 0)
			{
				//Broodwar->sendText("Constroi um pylon!");
				pylon++;
				ger_producao.push_back(UnitTypes::Protoss_Pylon);
				primeira = false;
			}
			if (workers >= 6 && gateways < 2 && gateway <= 1)
			{
				
				UnitType gatewayType = UnitTypes::Protoss_Gateway;
				//UnitType supplyProviderType = hq->getType().getRace().getSupplyProvider();
				//Broodwar->self()->
				//Broodwar->self()->

				/*for (std::list<Unit>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++){
					if ((*i)->getType() == UnitTypes::Protoss_Pylon){
						if (!(*i)->isCompleted() || Broodwar->self()->minerals() < 150) break;
						for (std::list<Unit>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++){
							if ((*i)->getType().isWorker() == true){
								TilePosition targetBuildLocation = Broodwar->getBuildLocation(gateway, (*i)->getTilePosition());
								(*i)->build(gateway, targetBuildLocation);
								Broodwar->sendText("Controi pae!");
								break;
							}
						}
						break;
					}
				}*/
				//Broodwar->sendText("Constroi um gateway!");
				
				gateway++;
				ger_producao.push_back(gatewayType);

			}
			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}
		}
		Sleep(20);
	}
}

DWORD WINAPI AAExample::Building_Pylon(LPVOID param)
{
	BWAPI::Unit unit = static_cast<BWAPI::Unit>(param);
	DWORD dwWaitResult;

	while (true)
	{
		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		if (GameOver || unit == NULL || !unit->exists())  {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		}

		if (!unit->isCompleted() || !unit->isCompleted()){ // You can create it on the onUnitComplete too!
			ReleaseMutex(ghMutex);
			Sleep(500);
			continue;
		}

		if (unit->isCompleted())
		{
			pylon--;
			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}
			break;
		}

		if (!ReleaseMutex(ghMutex))
		{
			// Handle error.
		}

		Sleep(20);
	}

	/*if (!ReleaseMutex(ghMutex))
	{
		// Handle error.
	}*/

	return dwWaitResult;
}

DWORD WINAPI AAExample::Building_Gateway(LPVOID param)
{
	BWAPI::Unit unit = static_cast<BWAPI::Unit>(param);
	DWORD dwWaitResult;

	while (true)
	{
		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		if (GameOver || unit == NULL || !unit->exists())  {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		}

		if (!unit->isCompleted() || !unit->isCompleted()){ // You can create it on the onUnitComplete too!
			ReleaseMutex(ghMutex);
			Sleep(500);
			continue;
		}

		if (unit->isCompleted())
		{
			gateway--;
			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}
			break;
		}

		if (!ReleaseMutex(ghMutex))
		{
			// Handle error.
		}

		Sleep(20);
	}

	/*if (!ReleaseMutex(ghMutex))
	{
		// Handle error.
	}*/

	return dwWaitResult;
}

DWORD WINAPI AAExample::Zealot(LPVOID param)
{
	BWAPI::Unit unit = static_cast<BWAPI::Unit>(param);
	DWORD dwWaitResult;
	bool completing = true;

	while (true)
	{
		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		if (GameOver || unit == NULL || !unit->exists())  {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		}

		if (!unit->isCompleted() || !unit->isCompleted()){ // You can create it on the onUnitComplete too!
			ReleaseMutex(ghMutex);
			Sleep(500);
			continue;
		}

		if (completing/*unit->isCompleted()*/)
		{
			unit_zealot++;
			completing = false;
		}

		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED) //RAII
		{
			/*if (unit->isIdle())
			{
				unit->
			}*/
			//Unit enemy = unit->getClosestUnit(/*!Filter::IsResourceDepot && !Filter::IsResourceContainer*//*GetType == UnitTypes::Men &&*/ Filter::IsEnemy /*&& Filter::IsAttacking*/);
			//if (enemy != NULL)
			//{
			//	UnitType asd = enemy->getType();
			//	unit->attack(enemy);
			//}
			//Broodwar << "Tipo do inimigo: " << enemy->getType() << std::endl;
			if ((vouAtacar/*Broodwar->enemy()->getUnits().size() > 0*/ && !unit->isMoving()) || unit->isUnderAttack())
			{
				bool encontrou = false;
				for (list<Unit>::const_iterator asd = Broodwar->enemy()->getUnits().begin(); asd != Broodwar->enemy()->getUnits().end(); asd++)
				{
					if ((*asd) != NULL && (!unit->isAttackFrame() || !unit->isAttacking() || !unit->isStartingAttack()))
					{
						if ((*asd)->getType() == UnitTypes::Men)
						{
							encontrou = true;
							unit->attack((*asd));
							break;
						}
					}
				}
				if (!encontrou)
				{
					for (list<Unit>::const_iterator asd = Broodwar->enemy()->getUnits().begin(); asd != Broodwar->enemy()->getUnits().end(); asd++)
					{
						if ((*asd) != NULL && (!unit->isAttackFrame() || !unit->isAttacking() || !unit->isStartingAttack()))
						{
							encontrou = true;
							unit->attack((*asd));
							break;
						}
					}
				}
				if (!encontrou && unit->isIdle())
				{
					unit->move((Position) pos_base_inimiga);
				}
			}

			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}

			Sleep(5);
		}
	}

	/*if (!ReleaseMutex(ghMutex))
	{
	// Handle error.
	}*/

	return dwWaitResult;
}

DWORD WINAPI AAExample::thisShouldBeAClassButImTooLazyToDoIt_Worker(LPVOID param){

	BWAPI::Unit unit = static_cast<BWAPI::Unit>(param);
	DWORD dwWaitResult;
	bool constructing = true;

	while (true){

		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		//// If end game, or if it exists (remember to always check)
		if (GameOver || unit == NULL || !unit->exists())  {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		} // end thread
		// You can check tons of others things like isStuck, isLockedDown, constructing
		if (!unit->isCompleted() || !unit->isCompleted()){ // You can create it on the onUnitComplete too!
			ReleaseMutex(ghMutex);
			Sleep(500);
			continue;
		}

		if (constructing)
		{
			trabalhadores[unit->getID()] = IDLE;
			workers++;
			probe--;
			constructing = false;
		}

		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED) //RAII
		{
			// if our worker is idle
			/*if (unit->isIdle())
			{
				// Order workers carrying a resource to return them to the center,
				// otherwise find a mineral patch to harvest.
				if (unit->isCarryingGas() || unit->isCarryingMinerals())
				{
					unit->returnCargo();
				}
				else if (!unit->getPowerUp())  // The worker cannot harvest anything if it
				{                             // is carrying a powerup such as a flag
					// Harvest from the nearest mineral patch or gas refinery
					BWAPI::Unit tempu = unit->getClosestUnit(
						BWAPI::Filter::IsMineralField || BWAPI::Filter::IsRefinery);
					if (tempu != NULL && !unit->gather(tempu))
					{
						// If the call fails, then print the last error message
						// Broodwar << Broodwar->getLastError() << std::endl;
					}

				} // closure: has no powerup
			} // closure: if idle
			*/
			
			//funfando
			map<int, int>::const_iterator it = trabalhadores.find(unit->getID());
			if (it != trabalhadores.end())
			{

				if (unit->isIdle())
				{
					if (it->second == COLLECTING_MINERAL)
					{
						if (unit->isCarryingGas() || unit->isCarryingMinerals())
						{
							unit->returnCargo();
						}
						else if (!unit->getPowerUp())  // The worker cannot harvest anything if it
						{                             // is carrying a powerup such as a flag
							// Harvest from the nearest mineral patch or gas refinery
							BWAPI::Unit tempu = unit->getClosestUnit(BWAPI::Filter::IsMineralField);
							if (tempu != NULL && !unit->gather(tempu))
							{
								// If the call fails, then print the last error message
								// Broodwar << Broodwar->getLastError() << std::endl;
							}

						}
					}
					else if (it->second == COLLECTING_GAS)
					{
						if (unit->isCarryingGas() || unit->isCarryingMinerals())
						{
							unit->returnCargo();
						}
						else if (!unit->getPowerUp())  // The worker cannot harvest anything if it
						{                             // is carrying a powerup such as a flag
							// Harvest from the nearest mineral patch or gas refinery
							BWAPI::Unit tempu = unit->getClosestUnit(BWAPI::Filter::IsRefinery);
							if (tempu != NULL && !unit->gather(tempu))
							{
								// If the call fails, then print the last error message
								// Broodwar << Broodwar->getLastError() << std::endl;
							}

						}
					}
					/*else if (it->second == BUILDING)
					{
					Broodwar->sendText("ta entrando aqui?");
					//construindo--
					if (build_list.empty())
					trabalhadores[unit->getID()] = IDLE;

					pair<TilePosition, UnitType> bl = build_list.back();
					unit->build(bl.second, bl.first);
					Error lastErr = Broodwar->getLastError();
					if (lastErr == Errors::None)
					{
					build_list.pop_back();
					}
					}*/
					//falta o resto ainda
					else
					{
						trabalhadores[unit->getID()] = IDLE;
					}
				}
				else if (it->second == SCOUT)
				{
					//Unit mineralField = unit->getClosestUnit(Filter::IsMineralField);
					//Unit refinery = unit->getClosestUnit(Filter::IsRefinery);
					Unit enemyDepot = unit->getClosestUnit(Filter::IsResourceDepot && Filter::IsEnemy);
					if (enemyDepot != NULL && ref_base_inimiga == NULL)
					{
						ref_base_inimiga = enemyDepot;
						pos_base_inimiga = enemyDepot->getTilePosition();
						//TilePosition topzera = enemyDepot->getTilePosition();
						//Position pos = Position(topzera);
						//int kk = 0;
						//Broodwar << "Base inimiga: " << ref_base_inimiga->getTilePosition() << std::endl;
					}
				}
				/*else if (it->second == BUILDING)
				{
				//Broodwar->sendText("ta entrando aqui 2?");
				//construindo--
				if (build_list.empty())
				{
				//construindo--;
				trabalhadores[unit->getID()] = IDLE;
				//Broodwar->sendText("ta entrando aqui 2?");
				}
				else
				{
				//Broodwar->sendText("construindo like a boss");
				pair<TilePosition, UnitType> bl = build_list.back();
				unit->build(bl.second, bl.first);
				Error lastErr = Broodwar->getLastError();
				if (lastErr == Errors::None)
				{
				build_list.pop_back();
				//trabalhadores[unit->getID()] = IDLE;
				//construindo--;
				Broodwar->sendText("aiai");
				}
				//Broodwar << Broodwar->getLastError() << std::endl;
				else
				{
				build_list.pop_back();
				build_list.push_back(make_pair(Broodwar->getBuildLocation(UnitTypes::Protoss_Gateway, unit->getTilePosition()), UnitTypes::Protoss_Gateway));
				//Broodwar->sendText("amostra o erro");
				}
				//else if (lastErr == Errors::Invalid_Tile_Position || lastErr == Errors::Unreachable_Location || lastErr == Errors::Unbuildable_Location || lastErr == Errors::Insufficient_Space)
				//	Broodwar->sendText("bugou ne danado");
				}
				}*/
			}

			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}

			Sleep(5); // Some agents can sleep more than others. 
		}
	}
}

DWORD WINAPI AAExample::GeneralOrManagerOrGerenteOrSomethingLikeThat(LPVOID param){

	BWAPI::Unit hq = static_cast<BWAPI::Unit>(param);
	DWORD dwWaitResult;

	while (true){

		dwWaitResult = WaitForSingleObject(
			ghMutex,    // handle to mutex
			100);  // time-out interval

		if (GameOver || hq == NULL || !hq->exists()) {
			ReleaseMutex(ghMutex);
			return 0; // end thread
		}
		// Some things are commom between units, so you can apply a little of OO here.

		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED)
		{
			/*if (hq->isIdle() && !hq->train(hq->getType().getRace().getWorker()))
			{
				// If that fails, get error
				Error lastErr = Broodwar->getLastError();

				// Retrieve the supply provider type in the case that we have run out of supplies
				UnitType supplyProviderType = hq->getType().getRace().getSupplyProvider();
				static int lastChecked = 0;

				// If we are supply blocked and haven't tried constructing more recently
				if (lastErr == Errors::Insufficient_Supply &&
					lastChecked + 400 < Broodwar->getFrameCount() &&
					Broodwar->self()->incompleteUnitCount(supplyProviderType) == 0)
				{
					lastChecked = Broodwar->getFrameCount();

					// Retrieve a unit that is capable of constructing the supply needed
					Unit supplyBuilder = hq->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
						(IsIdle || IsGatheringMinerals) &&
						IsOwned);
					// If a unit was found
					if (supplyBuilder)
					{
						if (supplyProviderType.isBuilding())
						{
							TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, supplyBuilder->getTilePosition());
							if (targetBuildLocation)
							{
								// Order the builder to construct the supply structure
								supplyBuilder->build(supplyProviderType, targetBuildLocation);
							}
						}
						else
						{
							// Train the supply provider (Overlord) if the provider is not a structure
							supplyBuilder->train(supplyProviderType);
						}
					} // closure: supplyBuilder is valid
				} // closure: insufficient supply
			}*/ // closure: failed to train idle unit
			// Release ownership of the mutex object
			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}
		}
		Sleep(20);
	}
}
