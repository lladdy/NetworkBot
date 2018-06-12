#include "LadderCoordinator.h"
#include "sc2api/sc2_server.h"

struct ConnectionOptions
{
	int32_t GamePort;
	int32_t StartPort;
	std::string ServerAddress;
	bool ComputerOpponent;
	sc2::Difficulty ComputerDifficulty;
	sc2::Race ComputerRace;
	std::string type;
};

bool SendDataToConnection(sc2::Connection *Connection, const SC2APIProtocol::Request *request)
{
	if (Connection->connection_ != nullptr)
	{
		Connection->Send(request);
		return true;
	}
	return false;
}

bool ProcessResponse(const SC2APIProtocol::ResponseCreateGame& response)
{
	bool success = true;
	if (response.has_error()) {
		std::string errorCode = "Unknown";
		switch (response.error()) {
		case SC2APIProtocol::ResponseCreateGame::MissingMap: {
			errorCode = "Missing Map";
			break;
		}
		case SC2APIProtocol::ResponseCreateGame::InvalidMapPath: {
			errorCode = "Invalid Map Path";
			break;
		}
		case SC2APIProtocol::ResponseCreateGame::InvalidMapData: {
			errorCode = "Invalid Map Data";
			break;
		}
		case SC2APIProtocol::ResponseCreateGame::InvalidMapName: {
			errorCode = "Invalid Map Name";
			break;
		}
		case SC2APIProtocol::ResponseCreateGame::InvalidMapHandle: {
			errorCode = "Invalid Map Handle";
			break;
		}
		case SC2APIProtocol::ResponseCreateGame::MissingPlayerSetup: {
			errorCode = "Missing Player Setup";
			break;
		}
		case SC2APIProtocol::ResponseCreateGame::InvalidPlayerSetup: {
			errorCode = "Invalid Player Setup";
			break;
		}
		default: {
			break;
		}
		}

		std::cerr << "CreateGame request returned an error code: " << errorCode << std::endl;
		success = false;
	}

	if (response.has_error_details() && response.error_details().length() > 0) {
		std::cerr << "CreateGame request returned error details: " << response.error_details() << std::endl;
		success = false;
	}
	return success;

}

void ResolveMap(const std::string& map_name, SC2APIProtocol::RequestCreateGame* request, sc2::ProcessSettings process_settings) {
	// BattleNet map
	if (!sc2::HasExtension(map_name, ".SC2Map")) {
		request->set_battlenet_map_name(map_name);
		return;
	}

	// Absolute path
	SC2APIProtocol::LocalMap* local_map = request->mutable_local_map();
	if (sc2::DoesFileExist(map_name)) {
		local_map->set_map_path(map_name);
		return;
	}

	// Relative path - Game maps directory
	std::string game_relative = sc2::GetGameMapsDirectory(process_settings.process_path) + map_name;
	if (sc2::DoesFileExist(game_relative)) {
		local_map->set_map_path(map_name);
		return;
	}

	// Relative path - Library maps directory
	std::string library_relative = sc2::GetLibraryMapsDirectory() + map_name;
	if (sc2::DoesFileExist(library_relative)) {
		local_map->set_map_path(library_relative);
		return;
	}

	// Relative path - Remotely saved maps directory
	local_map->set_map_path(map_name);
}

sc2::GameRequestPtr CreateStartGameRequest(std::string MapName, std::vector<sc2::PlayerSetup> players, sc2::ProcessSettings process_settings)
{
	sc2::ProtoInterface proto;
	sc2::GameRequestPtr request = proto.MakeRequest();

	SC2APIProtocol::RequestCreateGame* request_create_game = request->mutable_create_game();
	for (const sc2::PlayerSetup& setup : players)
	{
		SC2APIProtocol::PlayerSetup* playerSetup = request_create_game->add_player_setup();
		playerSetup->set_type(SC2APIProtocol::PlayerType(setup.type));
		playerSetup->set_race(SC2APIProtocol::Race(int(setup.race) + 1));
		playerSetup->set_difficulty(SC2APIProtocol::Difficulty(setup.difficulty));
	}
	ResolveMap(MapName, request_create_game, process_settings);

	request_create_game->set_realtime(false);
	return request;
}

void StartServer(int argc, char* argv[], ConnectionOptions &connect_options)
{
	sc2::ProcessSettings process_settings;
	sc2::GameSettings game_settings;
	sc2::ParseSettings(argc, argv, process_settings, game_settings);
	//uint64_t Bot1ProcessId = sc2::StartProcess(process_settings.process_path,
	//	{ "-listen", "0.0.0.0",
	//	"-port", std::to_string(connect_options.GamePort),
	//	"-displayMode", "0",
	//	"-dataVersion", process_settings.data_version }
	//);

	sc2::Connection client;
	client.Connect("127.0.0.1", connect_options.GamePort);
	int connectionAttemptsClient = 0;
	while (!client.Connect("127.0.0.1", connect_options.GamePort, false))
	{
		connectionAttemptsClient++;
		sc2::SleepFor(1000);
		if (connectionAttemptsClient > 60)
		{
			throw "Failed to connect client 1. BotProcessID: ";
		}
	}

	std::vector<sc2::PlayerSetup> Players;
	Players.push_back(sc2::PlayerSetup(sc2::PlayerType::Participant, sc2::Race::Terran, nullptr, sc2::Easy));
	Players.push_back(sc2::PlayerSetup(sc2::PlayerType::Computer, sc2::Race::Random, nullptr, sc2::Difficulty::Hard));
	sc2::GameRequestPtr Create_game_request = CreateStartGameRequest("InterloperLE.SC2Map", Players, process_settings);
	SendDataToConnection(&client, Create_game_request.get());

	SC2APIProtocol::Response* create_response = nullptr;
	if (client.Receive(create_response, 100000))
	{
		std::cout << "Recieved create game response " << create_response->data().DebugString() << std::endl;
		if (ProcessResponse(create_response->create_game()))
		{
			std::cout << "Create game successful" << std::endl << std::endl;
		}
	}
}

static sc2::Difficulty GetDifficultyFromString(std::string InDifficulty)
{
	if (InDifficulty == "VeryEasy")
	{
		return sc2::Difficulty::VeryEasy;
	}
	if (InDifficulty == "Easy")
	{
		return sc2::Difficulty::Easy;
	}
	if (InDifficulty == "Medium")
	{
		return sc2::Difficulty::Medium;
	}
	if (InDifficulty == "MediumHard")
	{
		return sc2::Difficulty::MediumHard;
	}
	if (InDifficulty == "Hard")
	{
		return sc2::Difficulty::Hard;
	}
	if (InDifficulty == "HardVeryHard")
	{
		return sc2::Difficulty::HardVeryHard;
	}
	if (InDifficulty == "VeryHard")
	{
		return sc2::Difficulty::VeryHard;
	}
	if (InDifficulty == "CheatVision")
	{
		return sc2::Difficulty::CheatVision;
	}
	if (InDifficulty == "CheatMoney")
	{
		return sc2::Difficulty::CheatMoney;
	}
	if (InDifficulty == "CheatInsane")
	{
		return sc2::Difficulty::CheatInsane;
	}

	return sc2::Difficulty::Easy;
}

static sc2::Race GetRaceFromString(const std::string & RaceIn)
{
	std::string race(RaceIn);
	std::transform(race.begin(), race.end(), race.begin(), ::tolower);

	if (race == "terran")
	{
		return sc2::Race::Terran;
	}
	else if (race == "protoss")
	{
		return sc2::Race::Protoss;
	}
	else if (race == "zerg")
	{
		return sc2::Race::Zerg;
	}
	else if (race == "random")
	{
		return sc2::Race::Random;
	}

	return sc2::Race::Random;
}

static void ParseArguments(int argc, char *argv[], ConnectionOptions &connect_options)
{
	sc2::ArgParser arg_parser(argv[0]);
	arg_parser.AddOptions({
		{ "-g", "--GamePort", "Port of client to connect to", false },
		{ "-o", "--StartPort", "Starting server port", false },
		{ "-l", "--LadderServer", "Ladder server address", false },
		{ "-c", "--ComputerOpponent", "If we set up a computer oppenent" },
		{ "-a", "--ComputerRace", "Race of computer oppent"},
		{ "-d", "--ComputerDifficulty", "Difficulty of computer oppenent" },
		{ "-t", "--Type", "Type of instance"}
		});
	arg_parser.Parse(argc, argv);
	std::string GamePortStr;
	if (arg_parser.Get("GamePort", GamePortStr)) {
		connect_options.GamePort = atoi(GamePortStr.c_str());
	}
	std::string StartPortStr;
	if (arg_parser.Get("StartPort", StartPortStr)) {
		connect_options.StartPort = atoi(StartPortStr.c_str());
	}
	arg_parser.Get("LadderServer", connect_options.ServerAddress);
	std::string CompOpp;
	if (arg_parser.Get("ComputerOpponent", CompOpp))
	{
		connect_options.ComputerOpponent = true;
		std::string CompRace;
		if (arg_parser.Get("ComputerRace", CompRace))
		{
			connect_options.ComputerRace = GetRaceFromString(CompRace);
		}
		std::string CompDiff;
		if (arg_parser.Get("ComputerDifficulty", CompDiff))
		{
			connect_options.ComputerDifficulty = GetDifficultyFromString(CompDiff);
		}

	}
	else
	{
		connect_options.ComputerOpponent = false;
	}
	arg_parser.Get("Type", connect_options.type);
}

//void RunGame(int argc, char *argv[])
//{
	//sc2::Coordinator coordinator;
	//if (!coordinator.LoadSettings(argc, argv)) {
	//	return;
	//}

	//coordinator.SetMultithreaded(true);
	//if (PlayerOneIsHuman) {
	//	coordinator.SetRealtime(true);
	//}

	//// Add the custom bot, it will control the players.
	//sc2::TerranBot bot1, bot2;
	//Human human_bot;

	//sc2::Agent* player_one = &bot1;
	//if (PlayerOneIsHuman) {
	//	player_one = &human_bot;
	//}

	//coordinator.SetParticipants({
	//	CreateParticipant(sc2::Race::Terran, player_one),
	//	CreateParticipant(sc2::Race::Terran, &bot2),
	//	});

	//// Start the game.
	//coordinator.LaunchStarcraft();

	//bool do_break = false;
	//while (!do_break) {
	//	if (!coordinator.StartGame(sc2::kMapBelShirVestigeLE)) {
	//		break;
	//	}
	//	while (coordinator.Update() && !do_break) {
	//		if (sc2::PollKeyPress()) {
	//			do_break = true;
	//		}
	//	}
	//}
//}

void RunGame(int argc, char* argv[], ConnectionOptions &Options)
{
	std::cout << "Creating server..." << std::endl << std::endl;

	// Setup server that mimicks sc2.
	sc2::Server server;
	server.Listen(std::to_string(Options.GamePort).c_str(), "100000", "100000", "5");

	std::cout << "Starting SC2..." << std::endl << std::endl;

	// Find game executable and run it.
	sc2::ProcessSettings process_settings;
	sc2::GameSettings game_settings;
	sc2::ParseSettings(argc, argv, process_settings, game_settings);
	uint64_t sc2ProcessId = sc2::StartProcess(process_settings.process_path,
		{ "-listen", "127.0.0.1",
		"-port", std::to_string(Options.GamePort + 2),
		"-displayMode", "0",
		"-dataVersion", process_settings.data_version }
	);

	std::cout << "Creating and connecting client..." << std::endl << std::endl;

	// Connect to running sc2 process.
	sc2::Connection client;
	//client.Connect("127.0.0.1", 5679);
	int connectionAttemptsClient = 0;
	while (!client.Connect("127.0.0.1", Options.GamePort+2, false))
	{
		connectionAttemptsClient++;
		sc2::SleepFor(1000);
		if (connectionAttemptsClient > 60)
		{
			throw "Failed to connect client 1. BotProcessID: ";
		}
	}

	if (Options.ServerAddress.length() == 0) {
		std::cout << "Operating as game host." << std::endl;
		std::cout << "Creating game..." << std::endl << std::endl;

		std::vector<sc2::PlayerSetup> Players;
		Players.push_back(sc2::PlayerSetup(sc2::PlayerType::Participant, sc2::Race::Terran, nullptr, sc2::Easy));
		Players.push_back(sc2::PlayerSetup(sc2::PlayerType::Participant, sc2::Race::Terran, nullptr, sc2::Easy));
		sc2::GameRequestPtr Create_game_request = CreateStartGameRequest("InterloperLE.SC2Map", Players, process_settings);
		client.Send(Create_game_request.get());
		SC2APIProtocol::Response* create_response = nullptr;
		if (client.Receive(create_response, 100000))
		{
			std::cout << "Recieved create game response " << create_response->data().DebugString() << std::endl;
			if (ProcessResponse(create_response->create_game()))
			{
				std::cout << "Create game successful" << std::endl << std::endl;
			}
		}
	}

	std::cout << "Press any key to quit..." << std::endl << std::endl;

	while (!sc2::PollKeyPress()) {
		// If the proxy has messages forward them to sc2.
		if (server.HasRequest()) {
			server.SendRequest(client.connection_);

			// Block for sc2's response then queue it.
			SC2APIProtocol::Response* response = nullptr;
			client.Receive(response, 100000);
			server.QueueResponse(client.connection_, response);

			std::cout << "Sending response" << std::endl;

			// Send the response back to the client.
			server.SendResponse();
		}
	}

	sc2::TerminateProcess(sc2ProcessId);
}

static void RunBot(int argc, char *argv[], sc2::Agent *Agent,sc2::Race race)
{
	ConnectionOptions Options;
	ParseArguments(argc, argv, Options);

	LadderCoordinator coordinator;
	if (!coordinator.LoadSettings(argc, argv)) {
		return;
	}

	// Add the custom bot, it will control the players.
	int num_agents;
	if (Options.ComputerOpponent)
	{
		num_agents = 1;
		coordinator.SetParticipants({
			CreateParticipant(race, Agent),
			CreateComputer(Options.ComputerRace, Options.ComputerDifficulty)
			});
	}
	else
	{
		num_agents = 2;
		coordinator.SetParticipants({
			CreateParticipant(race, Agent),
			});
	}

	// Start the game.

	// Step forward the game simulation.
	std::cout << "Connecting to port " << Options.GamePort << std::endl;
	coordinator.Connect("127.0.0.1", Options.GamePort);
	coordinator.SetupPorts(num_agents, Options.StartPort, false);
	// Step forward the game simulation.
	coordinator.JoinGame();
	coordinator.SetTimeoutMS(10000);
	std::cout << " Successfully joined game" << std::endl;
	while (coordinator.Update()) {
	}
}
