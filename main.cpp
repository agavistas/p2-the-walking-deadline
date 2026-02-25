#include <cstdint>
#include <string>
#include <deque>
#include <queue>
#include <iostream>
#include <getopt.h>
#include "P2random.h"

typedef struct Zombie {
	std::string name;
	uint32_t distance;
	uint32_t speed;
	uint32_t health;
	uint32_t lifetime;
} Zombie;

class Battle {
	public:
	Battle(int argc, char** argv) {
		int option_index = 0, option = 0;
		opterr = false;
		struct option longOpts[] = {
			{"verbose", no_argument, nullptr, 'v'},
			{"statistics", required_argument, nullptr, 's'},
			{"median", no_argument, nullptr, 'm'},
			{"help", no_argument, nullptr, 'h'},
			{nullptr, 0, nullptr, '\0'}};
		while ((option = getopt_long(argc, argv, "vs:mh\0", longOpts, &option_index)) != -1) {
			switch (option) {
				case 'v': verbose = true; break;
				case 's': stats = static_cast<uint32_t>(std::stoul(optarg)); break;
				case 'm': median = true; break;
				case 'h': printHelp(); exit(0);
			}
		} 	
	}

	void initialize() {
		std::string buffer;
		getline(std::cin, buffer, '\n');
		uint32_t random_seed, max_rand_distance, max_rand_speed, max_rand_health;
		std::cin >> buffer >> quiver_capacity;
		std::cin >> buffer >> random_seed;
		std::cin >> buffer >> max_rand_distance;
		std::cin >> buffer >> max_rand_speed;
		std::cin >> buffer >> max_rand_health;
		P2random::initialize(random_seed, max_rand_distance, max_rand_speed, max_rand_health);
	}

	void run() {
		uint32_t round = 1;
		{ std::string buffer;
		std::cin >> buffer >> buffer >> nextWave; }
		while (true) {
			if (verbose) std::cout << "Round: " << round << "\n";
			quiver = quiver_capacity;
			Zombie* endingZombie = moveZombies();
			if (endingZombie != nullptr) {
				std::cout << "DEFEAT IN ROUND " << round << "! " << endingZombie->name << " ate your brains!\n";
				if (stats > 0) printStats();
				return;
			}
			if (round == nextWave) addNewZombies();
			endingZombie = attackZombies();
			if (!moreRounds && endingZombie != nullptr) {
				std::cout <<"VICTORY IN ROUND " << round << "! " << endingZombie->name << " was the last zombie.\n";
				if (stats > 0) printStats();
				return;
			}
			++round;
		}
	}

	~Battle() {
		for (Zombie *zomb : zombies) delete(zomb);
	}

	private:
	void printStats() {
		std::cout << "Zombies still active: " << eta_queue.size() << "\nFirst zombies killed:\n";		
		auto iter = deadies.begin();
		for (uint32_t i = 0; i < stats; ++i) std::cout << (*(iter++))->name << " " << i+1 << "\n";
		std::cout << "Last zombies killed:\n";
		iter = --deadies.end();
		for (uint32_t i = 0; i < stats; ++i) std::cout << (*(iter--))->name << " " << deadies.size()-i << "\n";
	}

	Zombie* moveZombies() {
		Zombie* ptr = nullptr;
		for (Zombie* zomb : zombies) if (zomb->health) {
			++(zomb->lifetime);
			zomb->distance -= std::min(zomb->distance, zomb->speed);
			if (zomb->distance == 0 && !ptr) ptr = zomb;
			if (verbose) std::cout << 
				"Moved: " << zomb->name << " (distance: " << zomb->distance
				<< ", speed: " << zomb->speed << ", health: " << zomb->health << ")\n"; 
		}
		return ptr;
	}

	Zombie* attackZombies() {
		for (; quiver > 0; --quiver) {
			Zombie* top = eta_queue.top();
			--(top->health);
			if (!top->health) {
				eta_queue.pop();
				deadies.push_back(top);
				if (verbose) std::cout << 
				"Destroyed: " << top->name << " (distance: " << top->distance
				<< ", speed: " << top->speed << ", health: " << top->health << ")\n"; 
			}
			if (eta_queue.empty()) return top;
		}
		return nullptr;
	}

	void addNewZombies() {
		uint32_t newZombies;
		std::string buffer;
		std::cin >> buffer >> newZombies;
		for (; newZombies > 0; --newZombies) {
			Zombie* zombieptr = new Zombie();
			zombieptr->name = P2random::getNextZombieName();
			zombieptr->distance = P2random::getNextZombieDistance();
			zombieptr->speed = P2random::getNextZombieSpeed();
			zombieptr->health = P2random::getNextZombieHealth();
			zombieptr->lifetime = 0;
			zombies.push_back(zombieptr);
			eta_queue.push(zombieptr);

			if (verbose) std::cout << 
				"Created: " << zombieptr->name << " (distance: " << zombieptr->distance
				<< ", speed: " << zombieptr->speed << ", health: " << zombieptr->health << ")\n"; 
		}
		std::cin >> buffer >> newZombies;
		for (; newZombies > 0; --newZombies) {
			Zombie* zombieptr = new Zombie();
			std::cin >> zombieptr->name >> buffer >> zombieptr->distance >> 
			buffer >> zombieptr->speed >> buffer >> zombieptr->health;
			zombieptr->lifetime = 0;
			zombies.push_back(zombieptr);
			eta_queue.push(zombieptr);

			if (verbose) std::cout << 
				"Created: " << zombieptr->name << " (distance: " << zombieptr->distance
				<< ", speed: " << zombieptr->speed << ", health: " << zombieptr->health << ")\n"; 
		}
		if (std::cin >> buffer) std::cin >> buffer >> nextWave;
		else moreRounds = false;
	}

	struct etaGreater {
		bool operator()(const Zombie* left, const Zombie* right) {
			if ((left->distance / left->speed) != (right->distance / right->speed)) 
				return (left->distance / left->speed) > (right->distance / right->speed);
			if (left->health != right->health) return left->health > right->health;
			return left->name > right->name;
		}
	};

	// game related variables
	std::deque<Zombie*> zombies;
	std::deque<Zombie*> deadies;
	std::priority_queue<Zombie*, std::vector<Zombie*>, etaGreater> eta_queue;
	bool moreRounds = true;
	uint32_t nextWave;
	uint32_t quiver;
	
	// init related variables
	uint32_t quiver_capacity;

	// option related variables
	void printHelp() {};
	bool verbose = false;
	bool median = false;
	uint32_t stats = 0;
};

int main(int argc, char** argv) {
	std::ios::sync_with_stdio(false);
	Battle game(argc, argv);
	game.initialize();
	game.run();
}
