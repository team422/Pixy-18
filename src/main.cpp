// Workaround for working with GCC 5.4. Do not remove
//#define _GLIBCXX_USE_CXX11_ABI 0

#include <cscore.h>
#include <networktables/NetworkTable.h>
#include <tables/ITable.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <llvm/StringRef.h>
#include <llvm/ArrayRef.h>
#include <thread>
#include <string>
#include <chrono>
#include <vector>
#include <iostream>
#include <pixy.h>

typedef struct Block Block;

int main() {
	std::cout << "Starting program" << std::endl;
	NetworkTable::SetClientMode();
	NetworkTable::SetTeam(422);
	NetworkTable::SetIPAddress("10.4.22.2");
	NetworkTable::Initialize();
	
	std::shared_ptr<NetworkTable> pixy_table = NetworkTable::GetTable("pixy");
	pixy_table->PutBoolean("active", true);
	
	const uint32_t MAX_BLOCKS = 25;
	// An array to store the blocks in
	Block blocks[MAX_BLOCKS];

	int pixy_init_status = pixy_init();
	
	if (pixy_init_status != 0) {
		std::cout << "Failed at pixy init: ";
		pixy_error(pixy_init_status);
		return pixy_init_status;
	}
	
	std::cout << "Starting to transfer data..." << std::endl;
	
	char buffer[128];

	while (true) {
		while(!pixy_blocks_are_new()) {/* Wait for new data */}
		int number_of_blocks = pixy_get_blocks(MAX_BLOCKS, &blocks[0]);
		
		if (number_of_blocks < 0) {
			std::cout << "Failed while fetching blocks: ";
			pixy_error(pixy_init_status);
			continue;
		}

		if (number_of_blocks == 0) {
			continue;
		}
		
		Block *target = nullptr;		
		int current_area = 0;
		
		// Filter the blocks for what we want...
		for (int i = 0; i != number_of_blocks; ++i) {
			int area = blocks[i].width * blocks[i].height;
			if (area > current_area) {
				current_area = area;
				target = &blocks[i];
			}
		}

		if (target != nullptr) {
			std::cout << "Loading data to network tables" << std::endl;
			target->print(buffer);
			pixy_table->PutNumber("blockX", target->x);
			pixy_table->PutNumber("blockY", target->y);
			pixy_table->PutNumber("blockW", target->width);
			pixy_table->PutNumber("blockH", target->height);
			pixy_table->PutNumber("blockArea", target->width * target->height);
		}
	}
	
	return 0;
}
