#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <limits>

using std::cout;
using std::endl;
using std::string;
using std::vector;

string inputtracename = "trace.txt";
// remove the ".txt" and add ".out.txt" to the end as output name
string outputtracename = inputtracename.substr(0, inputtracename.length() - 4) + ".out.txt";
string hardwareconfigname = "config.txt";

enum Operation
{
	ADD,	//0
	SUB,	//1
	MULT,	//2
	DIV,	//3
	LOAD,	//4
	STORE	//5
};
// The execute cycle of each operation: ADD, SUB, MULT, DIV, LOAD, STORE
const int OperationCycle[6] = {2, 2, 10, 40, 2, 2};

struct HardwareConfig
{
	int LoadRSsize;	 // number of load reservation stations
	int StoreRSsize; // number of store reservation stations
	int AddRSsize;	 // number of add reservation stations
	int MultRSsize;	 // number of multiply reservation stations
	int FRegSize;	 // number of fp registers
};

// We use the following structure to record the time of each instruction
struct InstructionStatus
{
	int cycleIssued;
	int cycleExecuted; // execution completed
	int cycleWriteResult = 0;
};


/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
//string op/destination/op1/op2
struct Instruction
{
	string op;
	string destinationRegister;
	string sourceOprand1;
	string sourceOprand2;
	
};

class Instructions
{
public:
	void addInstruction(Instruction instruction)
	{
		_instructions.push_back(instruction);
	}
	
	//constructor
	Instructions(){}
private:
	vector<Instruction> _instructions;
};

// Register Result Status structure
struct RegisterResultStatus
{
	string ReservationStationName;
	bool dataReady;
	int index;

	int ReservationStation_index;
	int instruction_index;

	//constructor
	RegisterResultStatus(int i) : dataReady(false), index(i){}
};

class RegisterResultStatuses
{
public:
	// ...
	
	RegisterResultStatuses(int FRegSize)
	{
		for(int i=0; i < FRegSize; ++i)
		{
			_registers.push_back(RegisterResultStatus(i));
		}
	}

	void updateRegisterResultStatus(int index, string reservationStationName, bool dataReady, int issued_RS_index, int instruction_index)
	{
		
		if (index >= 0 && index < _registers.size())
        {
            _registers[index].ReservationStationName = reservationStationName;
            _registers[index].dataReady = dataReady;
			_registers[index].ReservationStation_index = issued_RS_index;
			_registers[index].instruction_index = instruction_index;
        }
        else
        {
            // ERROR
            std::cerr << "Invalid index for updating register result status." << std::endl;
        }
	}

	void updateSTORERegisterResultStatus (int index, bool dataReady, int issued_RS_index, int instruction_index)
	{
		if (index >= 0 && index < _registers.size())
        {
            _registers[index].dataReady = dataReady;
			_registers[index].ReservationStation_index = issued_RS_index;
			_registers[index].instruction_index = instruction_index;
        }
        else
        {
            // ERROR
            std::cerr << "Invalid index for updating register result status." << std::endl;
        }
	}

/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/
	/*
	Print all register result status. It is called by PrintRegisterResultStatus4Grade() for grading.
	If you don't want to write such a class, then make sure you call the same function and print the register
	result status in the same format.
	*/
	string _printRegisterResultStatus() const
	{
		std::ostringstream result;
		for (int idx = 0; idx < _registers.size(); idx++)
		{
			result << "F" + std::to_string(idx) << ": ";
			result << _registers[idx].ReservationStationName << ", ";
			result << "dataRdy: " << (_registers[idx].dataReady ? "Y" : "N") << ", ";
			result << "\n";
		}
		return result.str();
	}
/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
	auto begin() { return _registers.begin(); }
    auto end() { return _registers.end(); }
// private:
	vector<RegisterResultStatus> _registers;
};

// Define your Reservation Station structure
struct ReservationStation
{
	// ...
	//Station Name
	string ReservationStationName;

	//Station's op type (ADD, MULT, STORE, LOAD)
	Operation Station_type;

	//Station's type index (according to config)
	int Type_index;

	//Operation that is handleing
	Operation Op;
	
	int Vj;
	int Vk;

	string Qj;
	string Qk;

	int Remain_cycles;

	bool Busy = false;

	int result;

	int cycleIssued;

	int instruction_index;

	int ReservationStation_index;

	//constructor
	ReservationStation(string station_name, Operation station_type, int type_index, int station_index):
	ReservationStationName(station_name), Station_type(station_type), Type_index(type_index), ReservationStation_index(station_index){}

};
class ReservationStations
{
public:
	// ...
	//Create ReservationStations by the config
	ReservationStations(int LoadRsize, int StoreRsize, int AddRsize, int MultRsize)
	{
		int index = 0;
		//Create load stations
		for(int i = 0; i < LoadRsize; ++i)
		{
			_stations.push_back(ReservationStation("Load"+std::to_string(i), Operation::LOAD, i, index));
			index++;
		}
		//Create store stations
		for(int i = 0; i < StoreRsize; ++i)
		{
			_stations.push_back(ReservationStation("Store"+std::to_string(i), Operation::STORE, i, index));
			index++;
		}
		//Create add stations
		for(int i = 0; i < AddRsize; ++i)
		{
			_stations.push_back(ReservationStation("Add"+std::to_string(i), Operation::ADD, i, index));
			index++;
		}
		//Create mult stations
		for(int i = 0; i < MultRsize; ++i)
		{
			_stations.push_back(ReservationStation("Mult"+std::to_string(i), Operation::MULT, i, index));
			index++;
		}

	}
	string issueReservationStation(Operation station_type, Operation op, string qj, string qk, int cycleIssued, int instruction_index)
	{
		//find an avaliable Reservation Station
		for(auto& station : _stations)
		{
			// If have free Reservation Station
			if (station.Station_type == station_type && !station.Busy)
			{
				// Issue the Reservation Station
				station.Op = op;
				// station.Vj = vj;
				// station.Vk = vk;
				station.Qj = qj;
				station.Qk = qk;
				station.Remain_cycles = OperationCycle[op];
				station.Busy = true;
				station.cycleIssued = cycleIssued;
				station.instruction_index = instruction_index;
				return station.ReservationStationName; // Found and issued, return the specific RS
			}
		}
		// If no free RS avaliable
		return "";

	}

	string _printReservationStationStatus() const
	{
		std::ostringstream result;
		for (int idx = 0; idx < _stations.size(); idx++)
		{
			result << "RS" + std::to_string(idx) << ": ";
			result << _stations[idx].ReservationStationName << ", ";
			result << _stations[idx].Op << ", ";
			result << "Qj:" << _stations[idx].Qj << ", ";
			result << "Qk:" << _stations[idx].Qk << ", ";
			result << "Remain_cycles: " <<_stations[idx].Remain_cycles << ", ";
			result << "Busy: " << (_stations[idx].Busy ? "Y" : "N") << ", ";
			result << "\n";
		}
		return result.str();
	}

	auto begin() { return _stations.begin(); }
    auto end() { return _stations.end(); }

// private:
	vector<ReservationStation> _stations;
};

struct CDBMessage
{
	
	int value;
	string source;
	int cycleIssued;
	int ins_index;
};

class CommonDataBus
{
public:
	// ...
	void broadcast(int value, string source, int cycleIssued, int ins_index)
	{
		CDBMessage message{value, source, cycleIssued, ins_index};

		auto it = std::lower_bound(messages.begin(), messages.end(), message,
            [](const CDBMessage& lhs, const CDBMessage& rhs) {
                return lhs.cycleIssued < rhs.cycleIssued;
            });

		messages.insert(it, message);
	}

	vector<CDBMessage> messages;
};

/*
print the instruction status, the reservation stations and the register result status
@param filename: output file name
@param instructionStatus: instruction status
*/
void PrintResult4Grade(const string &filename, const vector<InstructionStatus> &instructionStatus)
{
	std::ofstream outfile(filename, std::ios_base::app); // append result to the end of file
	outfile << "Instruction Status:\n";
	for (int idx = 0; idx < instructionStatus.size(); idx++)
	{
		outfile << "Instr" << idx << ": ";
		outfile << "Issued: " << instructionStatus[idx].cycleIssued << ", ";
		outfile << "Completed: " << instructionStatus[idx].cycleExecuted << ", ";
		outfile << "Write Result: " << instructionStatus[idx].cycleWriteResult << ", ";
		outfile << "\n";
	}
	outfile.close();
}

/*
print the register result status each 5 cycles
@param filename: output file name
@param registerResultStatus: register result status
@param thiscycle: current cycle
*/
void PrintRegisterResultStatus4Grade(const string &filename,
									 const RegisterResultStatuses &registerResultStatus,
									 const ReservationStations &reservationStations,
									 const int thiscycle)
{
	// DEBUG
	if (thiscycle % 5 != 0)
		return;

	std::ofstream outfile(filename, std::ios_base::app); // append result to the end of file
	outfile << "Cycle " << thiscycle << ":\n";
	outfile << registerResultStatus._printRegisterResultStatus() << "\n";
	// DEBUG
	// outfile << reservationStations._printReservationStationStatus() << "\n";
	outfile.close();
}

// Function to simulate the Tomasulo algorithm
vector<InstructionStatus> simulateTomasulo(RegisterResultStatuses registerResultStatuses, vector<InstructionStatus> instructionStatuses, ReservationStations reservationStations, CommonDataBus CDB, vector<Instruction> instructions,
					  int loadRSsize, int storeRSsize, int addRSsize, int multRSsize, int reg_len, int ins_number)
{
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
	int thiscycle = 1; // start cycle: 1
	int instruction_index = 0;
	int instructions_length = ins_number;
	Operation INS_STATION_TYPE, INS_OP_TYPE;
	int INS_Destination_Register;
	string write_qk, write_qj, issued_RS_name;
	bool STOP, erase, found;
	// RegisterResultStatuses registerResultStatus;

	// while (thiscycle < 100000000)
	while (thiscycle < 100000000)
	{
		// Reservation Stations should be updated every cycle, and broadcast to Common Data Bus
		// ...
		// WB part
		// Read from CDB: reservationStation
		found = 0;
		erase = 0;
		if (!CDB.messages.empty()) 
		{
			// WE MUST ERASE EVERY CYCLE IF NOT EMPTY
			erase = 1;
			for(auto& reservationStation : reservationStations)
			{
				if(reservationStation.Qj == CDB.messages[0].source)
				{
					reservationStation.Vj = CDB.messages[0].value;
					reservationStation.Qj = "";
					erase = 1;
					// If new WB: wait for another cycle;
					if(reservationStation.Qj == "" && reservationStation.Qk == "")
					{
						++reservationStation.Remain_cycles;
					}
					int RS_index;
					string name1;
					name1 = CDB.messages[0].source;
					// Free the reservation station 
					if(CDB.messages[0].source[0] == 'L')
					{
						RS_index = std::stoi(name1.erase(0,4));
					}
					else if (CDB.messages[0].source[0] == 'S')
					{
						RS_index = loadRSsize + std::stoi(name1.erase(0,5));
					}
					else if (CDB.messages[0].source[0] == 'A')
					{
						RS_index = loadRSsize + storeRSsize + std::stoi(name1.erase(0,3));
					}
					else if (CDB.messages[0].source[0] == 'M')
					{
						RS_index = loadRSsize + storeRSsize + addRSsize + std::stoi(name1.erase(0,4));
					}
					// cycleWriteResult
					instructionStatuses[reservationStations._stations[RS_index].instruction_index].cycleWriteResult = thiscycle;

					reservationStations._stations[RS_index].Busy = false;
					// CDB.messages.erase(CDB.messages.begin());
					found = 1;

				}
				if (reservationStation.Qk == CDB.messages[0].source)
				{
					reservationStation.Vk = CDB.messages[0].value;
					reservationStation.Qk = "";
					erase = 1;
					// If new WB: wait for another cycle;
					if(reservationStation.Qj == "" && reservationStation.Qk == "")
					{
						++reservationStation.Remain_cycles;
					}
					int RS_index;
					string name1;
					name1 = CDB.messages[0].source;
					// Free the reservation station 
					if(CDB.messages[0].source[0] == 'L')
					{
						RS_index = std::stoi(name1.erase(0,4));
					}
					else if (CDB.messages[0].source[0] == 'S')
					{
						RS_index = loadRSsize + std::stoi(name1.erase(0,5));
					}
					else if (CDB.messages[0].source[0] == 'A')
					{
						RS_index = loadRSsize + storeRSsize + std::stoi(name1.erase(0,3));
					}
					else if (CDB.messages[0].source[0] == 'M')
					{
						RS_index = loadRSsize + storeRSsize + addRSsize + std::stoi(name1.erase(0,4));
					}
					// cycleWriteResult
					instructionStatuses[reservationStations._stations[RS_index].instruction_index].cycleWriteResult = thiscycle;
					
					reservationStations._stations[RS_index].Busy = false;
					// CDB.messages.erase(CDB.messages.begin());
					found = 1;
				}
			}
		}
		
		// Read from CDB: registers
		if (!CDB.messages.empty()) 
		{
			for(auto& registerResultStatus : registerResultStatuses)
			{
				if(registerResultStatus.ReservationStationName == CDB.messages[0].source)
				{
					registerResultStatus.dataReady = true;
					reservationStations._stations[registerResultStatus.ReservationStation_index].Busy = false;
					// registerResultStatus.ReservationStationName = "";
					if (instructionStatuses[registerResultStatus.instruction_index].cycleWriteResult == 0)
					{
						instructionStatuses[registerResultStatus.instruction_index].cycleWriteResult = thiscycle;
					}
					
					// NOOOO CLear the RS in Register
					// registerResultStatus.ReservationStationName = "";
					// CDB.messages.erase(CDB.messages.begin());
					erase = 1;
					//found
					found = 1;
				}
			}
		}

		if (!CDB.messages.empty()) 
		{
			if (!found)
			{
				int RS_index;
				string name1;
				name1 = CDB.messages[0].source;
				// Free the reservation station 
				if(CDB.messages[0].source[0] == 'L')
				{
					RS_index = std::stoi(name1.erase(0,4));
				}
				else if (CDB.messages[0].source[0] == 'S')
				{
					RS_index = loadRSsize + std::stoi(name1.erase(0,5));
				}
				else if (CDB.messages[0].source[0] == 'A')
				{
					RS_index = loadRSsize + storeRSsize + std::stoi(name1.erase(0,3));
				}
				else if (CDB.messages[0].source[0] == 'M')
				{
					RS_index = loadRSsize + storeRSsize + addRSsize + std::stoi(name1.erase(0,4));
				}
				// Clear RS
				reservationStations._stations[RS_index].Busy = false;
				// registerResultStatus.ReservationStationName = "";
				instructionStatuses[CDB.messages[0].ins_index].cycleWriteResult = thiscycle;

			}
		}
		if (erase)
		{
			CDB.messages.erase(CDB.messages.begin());
		}
		


		// RS Update Part + INS_state_update if executed
		for(auto& reservationStation : reservationStations)
		{
			if(reservationStation.Qj == "" && reservationStation.Qk == "")
			{
				--reservationStation.Remain_cycles;
			}
			// Executed. Ready to broadcast to CDB from RS add to broadcast queue by issue time
			if (reservationStation.Remain_cycles == 0)
			{
				CDB.broadcast(reservationStation.result, reservationStation.ReservationStationName, reservationStation.cycleIssued, reservationStation.instruction_index);
				instructionStatuses[reservationStation.instruction_index].cycleExecuted = thiscycle;
			}
		}
		

		// Issue new instruction in each cycle
		// ...

		if(instruction_index < instructions_length)
		{
			// CHECK OP_TYPE/STATION_TYPE
			if(instructions[instruction_index].op == "LOAD")
			{
				INS_STATION_TYPE = LOAD;
				INS_OP_TYPE = LOAD;
			}
			if(instructions[instruction_index].op == "STORE")
			{
				INS_STATION_TYPE = STORE;
				INS_OP_TYPE = STORE;
			}
			if(instructions[instruction_index].op == "ADD" || instructions[instruction_index].op == "SUB")
			{
				INS_STATION_TYPE = ADD;
				if(instructions[instruction_index].op == "ADD")
				{
					INS_OP_TYPE = ADD;
				}
				else INS_OP_TYPE = SUB;
			}
			if(instructions[instruction_index].op == "MULT" || instructions[instruction_index].op == "DIV")
			{
				INS_STATION_TYPE = MULT;
				if(instructions[instruction_index].op == "MULT")
				{
					INS_OP_TYPE = MULT;
				}
				else INS_OP_TYPE = DIV;
			}

			// Initialize op1/op2
			write_qj = "";
			write_qk = "";

			// If oprand1 or oprand2 are register
			// Check register readiness
			if(instructions[instruction_index].sourceOprand1[0] == 'F')
			{
				string register_name = instructions[instruction_index].sourceOprand1;
				int register_index;
				register_name.erase(0,1);
				register_index = std::stoi(register_name);

				// If register not ready: write qj
				if(!registerResultStatuses._registers[register_index].dataReady)
				{
					// write_qj = instructions[instruction_index].sourceOprand1;
					write_qj = registerResultStatuses._registers[register_index].ReservationStationName;
				}
			}
			if(instructions[instruction_index].sourceOprand2[0] == 'F')
			{
				string register_name = instructions[instruction_index].sourceOprand2;
				int register_index;
				register_name.erase(0,1);
				register_index = std::stoi(register_name);

				// If register not ready: write qk
				if(!registerResultStatuses._registers[register_index].dataReady)
				{
					write_qk = registerResultStatuses._registers[register_index].ReservationStationName;
				}
			}
			// STORE_REGISTER: write to qk
			if(instructions[instruction_index].op == "STORE")
			{
				string source_register_name = instructions[instruction_index].destinationRegister;
				int register_index;
				source_register_name.erase(0,1);
				register_index = std::stoi(source_register_name);

				// If source register not ready:  write qk
				if(!registerResultStatuses._registers[register_index].dataReady)
				{
					write_qk = registerResultStatuses._registers[register_index].ReservationStationName;
				}
			}

			string INS_Destination_Register_name = instructions[instruction_index].destinationRegister;
			INS_Destination_Register_name.erase(0,1);
			INS_Destination_Register = std::stoi(INS_Destination_Register_name);

			
			issued_RS_name = reservationStations.issueReservationStation(INS_STATION_TYPE, INS_OP_TYPE, write_qj, write_qk, thiscycle, instruction_index);
			// If free RS founded issued successfully
			if(issued_RS_name != "")
			{
				int issued_RS_index = 0;
				string name;
				name = issued_RS_name;
				if(issued_RS_name[0] == 'L')
				{
					issued_RS_index = std::stoi(name.erase(0,4));
				}
				else if (issued_RS_name[0] == 'S')
				{
					issued_RS_index = loadRSsize + std::stoi(name.erase(0,5));
				}
				else if (issued_RS_name[0] == 'A')
				{
					issued_RS_index = loadRSsize + storeRSsize + std::stoi(name.erase(0,3));
				}
				else if (issued_RS_name[0] == 'M')
				{
					issued_RS_index = loadRSsize + storeRSsize + addRSsize + std::stoi(name.erase(0,4));
				}
				
				if (issued_RS_name[0] != 'S')
				{
					registerResultStatuses.updateRegisterResultStatus(INS_Destination_Register, issued_RS_name, false, issued_RS_index, instruction_index);
				}
				// else
				// {
				// 	registerResultStatuses.updateSTORERegisterResultStatus(INS_Destination_Register, false, issued_RS_index, instruction_index);
				// }
				
				instructionStatuses[instruction_index].cycleIssued = thiscycle;
				// if(instruction_index < instructions_length)
				++instruction_index;
			}
		}
		
		

		// At the end of this cycle, we need this function to print all registers status for grading
		PrintRegisterResultStatus4Grade(outputtracename, registerResultStatuses, reservationStations, thiscycle);

		// DEBUG
		// PrintResult4Grade(outputtracename, instructionStatuses);

		++thiscycle;


		// The simulator should stop when all instructions are finished.
		// ...

		
		// Finish when all register data ready
		// STOP = 1;
		// for(auto& registerResultStatus : registerResultStatuses)
		// {
		// 	if(!registerResultStatus.dataReady)
		// 	{
		// 		STOP = 0;
		// 	}
		// }

		// Finish when ALL RS cleared
		STOP = 1;
		for(auto& reservationStation : reservationStations)
		{
			if(reservationStation.Busy)
			{
				STOP = 0;
			}

		}
		if (STOP){return instructionStatuses;}
	}
	return instructionStatuses;
};

/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/



int main(int argc, char **argv)
{
	if (argc > 1)
	{
		hardwareconfigname = argv[1];
		inputtracename = argv[2];
	}

	HardwareConfig hardwareConfig;
	std::ifstream config;
	config.open(hardwareconfigname);
	config >> hardwareConfig.LoadRSsize;  // number of load reservation stations
	config >> hardwareConfig.StoreRSsize; // number of store reservation stations
	config >> hardwareConfig.AddRSsize;	  // number of add reservation stations
	config >> hardwareConfig.MultRSsize;  // number of multiply reservation stations
	config >> hardwareConfig.FRegSize;	  // number of fp registers
	config.close();

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/

	// Read instructions from a file (replace 'instructions.txt' with your file name)
	// ...
	std::ifstream trace;
	string line;
	trace.open(inputtracename);
	vector<Instruction> instructions;
	vector<InstructionStatus> instructionStatuses;
	int INS_count = 0;
	while(!trace.eof())
	{
		Instruction instruction;
		InstructionStatus instructionStatus;
		getline(trace, line);
		std::istringstream iss(line);
		iss >> instruction.op >> instruction.destinationRegister >> instruction.sourceOprand1 >> instruction.sourceOprand2;
		//If not "\n"
		if(instruction.op != "")
		{
			instructions.push_back(instruction);
			instructionStatuses.push_back(instructionStatus);
			INS_count++;
		}
		
	}
	
	// Initialize the Reservation Stations
	ReservationStations reservationStations(hardwareConfig.LoadRSsize, hardwareConfig.StoreRSsize, hardwareConfig.AddRSsize, hardwareConfig.MultRSsize);

	// Initialize the register result status
	RegisterResultStatuses registerResultStatuses(hardwareConfig.FRegSize);
	// ...

	// Initialize the instruction status table

	// ...

	// Initialize CDB
	CommonDataBus CDB;

	// Simulate Tomasulo:
	instructionStatuses = simulateTomasulo(registerResultStatuses, instructionStatuses, reservationStations, CDB, instructions, hardwareConfig.LoadRSsize, hardwareConfig.StoreRSsize, hardwareConfig.AddRSsize, hardwareConfig.MultRSsize, hardwareConfig.FRegSize, INS_count);

/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

	// At the end of the program, print Instruction Status Table for grading
	PrintResult4Grade(outputtracename, instructionStatuses);

	return 0;
}
