#pragma once
#include <BWAPI.h>

// Remember not to use "Broodwar" in any global class constructor!

class AAExample : public BWAPI::AIModule
{
public:
	// Virtual functions for callbacks, leave these as they are.
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(std::string text);
	virtual void onReceiveText(BWAPI::Player player, std::string text);
	virtual void onPlayerLeft(BWAPI::Player player);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit unit);
	virtual void onUnitEvade(BWAPI::Unit unit);
	virtual void onUnitShow(BWAPI::Unit unit);
	virtual void onUnitHide(BWAPI::Unit unit);
	virtual void onUnitCreate(BWAPI::Unit unit);
	virtual void onUnitDestroy(BWAPI::Unit unit);
	virtual void onUnitMorph(BWAPI::Unit unit);
	virtual void onUnitRenegade(BWAPI::Unit unit);
	virtual void onSaveGame(std::string gameName);
	virtual void onUnitComplete(BWAPI::Unit unit);
	// Everything below this line is safe to modify.

	
	static DWORD WINAPI thisShouldBeAClassButImTooLazyToDoIt_Worker(LPVOID param);
	static DWORD WINAPI GeneralOrManagerOrGerenteOrSomethingLikeThat(LPVOID param);
	static DWORD WINAPI Construcao(LPVOID param);
	static DWORD WINAPI Gerenciador_Producao(LPVOID param);
	static DWORD WINAPI Gerenciador_Trabalhador(LPVOID param);
	static DWORD WINAPI Produtor(LPVOID param);

	static DWORD WINAPI Building_Pylon(LPVOID param);
	static DWORD WINAPI Building_Gateway(LPVOID param);
};
