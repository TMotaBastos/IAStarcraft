#include "AAExample.h"
#include <iostream>
#include <Windows.h>

using namespace BWAPI;
using namespace Filter;
using namespace std;

static int ZEALOT_RUSH = 0;
static int DRAGOON_MODE = 1;

int mode;

// AA: Do a blackboard, don't use it directly! (or at least don't use it static, use a singleton)
static bool GameOver;
HANDLE ghMutex;
int workers;
int gateways;
vector<UnitType> ger_producao;
set< pair<int, UnitType> > lista_producao;
int probe;
int zealot;
int dragoon;
int gateway;
int pylon;

int lastChecked;
bool vouConstruir;

Unit ref_base;
vector<Unit> ref_gateways;
Unit ref_pylon;

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
	mode = ZEALOT_RUSH;
	lastChecked = 0;
	vouConstruir = false;

	// Create a mutex with no initial owner
	ghMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	// Hello World!
	Broodwar->sendText("My badass agent is ready for battle!");

	// Set the command optimization level so that common commands can be grouped
	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(2);

	//construcao
	CreateThread(NULL, 0, Produtor, (LPVOID)NULL, 0, NULL);
	CreateThread(NULL, 0, Gerenciador_Producao, (LPVOID)NULL, 0, NULL);
	CreateThread(NULL, 0, Gerenciador_Trabalhador, (LPVOID)NULL, 0, NULL);
	CreateThread(NULL, 0, Construcao, (LPVOID)NULL, 0, NULL);

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
	//Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
	//Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());

}

void AAExample::onSendText(std::string text)
{
	// Send the text to the game if it is not being processed.
	Broodwar->sendText("%s", text.c_str());
	// Make sure to use %s and pass the text as a parameter,
	// otherwise you may run into problems when you use the %(percent) character!
}

void AAExample::onReceiveText(BWAPI::Player player, std::string text)
{ // AA: You won't need it because we don't have a team game, but it can change.
	// Parse the received text
	Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
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
		Broodwar << "Nuclear Launch Detected at " << target << std::endl;
	}
	else
	{
		// Otherwise, ask other players where the nuke is!
		Broodwar->sendText("Where's the nuke?");
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

void AAExample::onUnitCreate(BWAPI::Unit unit)
{
	// For each created unit...
	if (unit->getType().isWorker()) // or  == BWAPI::UnitTypes::Terran_SCV if terran
	{
		CreateThread(NULL, 0, thisShouldBeAClassButImTooLazyToDoIt_Worker, (LPVOID)unit, 0, NULL);
		workers++;
		probe--;
		Broodwar->sendText("Trabalhadores: %d", workers);
	}
	// You can do a direct comparison like  == BWAPI::UnitTypes::Terran_Command_Center too.
	else if (unit->getType().isResourceDepot())
	{
		ref_base = unit;
		CreateThread(NULL, 0, GeneralOrManagerOrGerenteOrSomethingLikeThat, (LPVOID)unit, 0, NULL);
	}
	else if (unit->getType() == UnitTypes::Protoss_Pylon)
	{
		vouConstruir = false;
		ref_pylon = unit;
		//pylon--;
		CreateThread(NULL, 0, Building_Pylon, (LPVOID)unit, 0, NULL);
		Broodwar->sendText("Pylon criado");
	}
	else if (unit->getType() == UnitTypes::Protoss_Gateway)
	{
		vouConstruir = false;
		ref_gateways.push_back(unit);
		gateways++;
		//gateway--;
		CreateThread(NULL, 0, Building_Gateway, (LPVOID)unit, 0, NULL);
		Broodwar->sendText("Gateways: %d", gateways);
	}
	else if (unit->getType() == UnitTypes::Protoss_Zealot)
		zealot--;
	else if (unit->getType() == UnitTypes::Protoss_Dragoon)
		dragoon--;
}

void AAExample::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType().isWorker())
	{
		Broodwar->sendText("Morreu!");
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

DWORD WINAPI AAExample::Produtor(LPVOID param)
{

	DWORD dwWaitResult;

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
					if ((*it).second.supplyRequired() > (Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed()))
					{
						++it;
						continue;
					}

					if ((*it).second == UnitTypes::Protoss_Probe && !vouConstruir)
					{
						if (ref_base->isIdle())
						{
							Broodwar->sendText("Probe - Minerio = %d", minerioAtual);
							minerioAtual -= (*it).second.mineralPrice();
							gasAtual -= (*it).second.gasPrice();
							ref_base->train((*it).second);
							//++it;
							it = lista_producao.erase(it);
						}
					}
					else if (((*it).second == UnitTypes::Protoss_Zealot || (*it).second == UnitTypes::Protoss_Dragoon) && !vouConstruir)
					{
						for (int i = 0; i < ref_gateways.size(); i++)
						{
							if (ref_gateways[i]->isIdle())
							{
								minerioAtual -= (*it).second.mineralPrice();
								gasAtual -= (*it).second.gasPrice();
								ref_gateways[i]->train((*it).second);
								//++it;
								it = lista_producao.erase(it);
								break;
							}
						}
					}
					else
					{
						//Broodwar->sendText("Tamanho da lista = %d", lista_producao.size());
						for (std::list<Unit>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++)
						{
							//if ((*i)->getType() == (*it).second.whatBuilds().first)
							if ((*i)->getType().isWorker() == true)
							{
								//Broodwar->sendText("Pylon - Minerio = %d", minerioAtual);
								minerioAtual -= (*it).second.mineralPrice();
								gasAtual -= (*it).second.gasPrice();
								//Broodwar->sendText("vai construir o pylon!");
								TilePosition targetBuildLocation = Broodwar->getBuildLocation((*it).second, (*i)->getTilePosition());
								//if (targetBuildLocation)
								//{
									/*if (*/(*i)->build((*it).second, targetBuildLocation);//)
								Error lastErr = Broodwar->getLastError();
								if (lastErr == Errors::None)
								{
									vouConstruir = true;
									it = lista_producao.erase(it);
								}

								//}
								break;
							}
						}
					}
				}
				else
				{
					++it;
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

			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}
		}
		Sleep(20);
	}
}

DWORD WINAPI AAExample::Gerenciador_Producao(LPVOID param)
{

	DWORD dwWaitResult;

	Broodwar->sendText("Comecou gerenciador de producao!");
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

			if ((workers < 9 && probe == 0) || probe == 0)
			{
				//Broodwar->sendText("Constroi um brodinho!");
				lista_producao.insert(make_pair(6, UnitTypes::Protoss_Probe));
				probe++;
			}
			else if (mode == ZEALOT_RUSH && zealot < gateways)
			{
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
			if (Broodwar->self()->supplyUsed() >= (Broodwar->self()->supplyTotal() - 1) && pylon == 0)
			{
				Broodwar->sendText("Constroi um pylon!");
				pylon++;
				ger_producao.push_back(UnitTypes::Protoss_Pylon);
			}
			if (workers >= 9 && gateways < 2 && gateway == 0)
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

		if (unit->isCompleted())
		{
			pylon--;
			break;
		}

		if (!ReleaseMutex(ghMutex))
		{
			// Handle error.
		}

		Sleep(20);
	}

	if (!ReleaseMutex(ghMutex))
	{
		// Handle error.
	}

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

		if (unit->isCompleted())
		{
			gateway--;
			break;
		}

		if (!ReleaseMutex(ghMutex))
		{
			// Handle error.
		}

		Sleep(20);
	}

	if (!ReleaseMutex(ghMutex))
	{
		// Handle error.
	}

	return dwWaitResult;
}

DWORD WINAPI AAExample::thisShouldBeAClassButImTooLazyToDoIt_Worker(LPVOID param){

	BWAPI::Unit unit = static_cast<BWAPI::Unit>(param);
	DWORD dwWaitResult;

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

		if (dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_ABANDONED) //RAII
		{
			// if our worker is idle
			if (unit->isIdle())
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

			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
			}

			Sleep(10); // Some agents can sleep more than others. 
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
