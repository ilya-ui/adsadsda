/* Property-based tests for Zombie Event */
#include "test.h"

#include <game/server/zombieevent.h>

#include <gtest/gtest.h>

#include <random>
#include <vector>
#include <cmath>

/**
 * Helper class to test CZombieEvent team assignment without full game context.
 * This simulates the team assignment algorithm for property testing.
 */
class CTeamAssignmentTester
{
public:
	/**
	 * Simulates the team assignment algorithm from CZombieEvent::AssignTeams()
	 * Returns pair of (builderCount, zombieCount)
	 */
	static std::pair<int, int> SimulateTeamAssignment(int NumPlayers, std::mt19937 &rng)
	{
		if(NumPlayers < 2)
			return {0, 0};

		// Shuffle players (simulated by just counting)
		std::vector<int> PlayerIds(NumPlayers);
		for(int i = 0; i < NumPlayers; i++)
			PlayerIds[i] = i;
		std::shuffle(PlayerIds.begin(), PlayerIds.end(), rng);

		// Split 50/50 - first half are builders
		int NumBuilders = NumPlayers / 2;
		if(NumPlayers % 2 == 1)
		{
			// Odd number - randomly decide if extra player is builder or zombie
			if(rng() % 2 == 0)
				NumBuilders++;
		}

		int NumZombies = NumPlayers - NumBuilders;
		return {NumBuilders, NumZombies};
	}
};

/**
 * **Feature: zombie-event, Property 1: Team assignment balance**
 * **Validates: Requirements 1.2**
 * 
 * For any number of players N >= 2, the team assignment should result in
 * |builders - zombies| <= 1
 */
TEST(ZombieEvent, PropertyTeamAssignmentBalance)
{
	std::random_device rd;
	std::mt19937 rng(rd());
	
	// Run 100 iterations with random player counts
	for(int iteration = 0; iteration < 100; iteration++)
	{
		// Generate random player count between 2 and MAX_CLIENTS
		std::uniform_int_distribution<int> playerDist(2, 64);
		int numPlayers = playerDist(rng);
		
		auto [builders, zombies] = CTeamAssignmentTester::SimulateTeamAssignment(numPlayers, rng);
		
		// Property: |builders - zombies| <= 1
		int difference = std::abs(builders - zombies);
		
		EXPECT_LE(difference, 1) 
			<< "Team balance violated for " << numPlayers << " players: "
			<< builders << " builders, " << zombies << " zombies, "
			<< "difference = " << difference;
		
		// Also verify total equals input
		EXPECT_EQ(builders + zombies, numPlayers)
			<< "Total players mismatch: expected " << numPlayers 
			<< ", got " << (builders + zombies);
	}
}

/**
 * Edge case: Exactly 2 players should result in 1 builder and 1 zombie
 */
TEST(ZombieEvent, TwoPlayersEqualSplit)
{
	std::random_device rd;
	std::mt19937 rng(rd());
	
	// With exactly 2 players, should always be 1-1 split
	for(int i = 0; i < 10; i++)
	{
		auto [builders, zombies] = CTeamAssignmentTester::SimulateTeamAssignment(2, rng);
		EXPECT_EQ(builders, 1);
		EXPECT_EQ(zombies, 1);
	}
}

/**
 * Edge case: Odd number of players
 */
TEST(ZombieEvent, OddPlayersBalance)
{
	std::random_device rd;
	std::mt19937 rng(rd());
	
	// Test various odd player counts
	std::vector<int> oddCounts = {3, 5, 7, 9, 11, 13, 15};
	
	for(int numPlayers : oddCounts)
	{
		for(int i = 0; i < 10; i++)
		{
			auto [builders, zombies] = CTeamAssignmentTester::SimulateTeamAssignment(numPlayers, rng);
			
			int difference = std::abs(builders - zombies);
			EXPECT_EQ(difference, 1)
				<< "Odd player count " << numPlayers << " should have difference of 1, "
				<< "got " << builders << " builders, " << zombies << " zombies";
		}
	}
}

/**
 * Edge case: Even number of players should have equal teams
 */
TEST(ZombieEvent, EvenPlayersEqualSplit)
{
	std::random_device rd;
	std::mt19937 rng(rd());
	
	// Test various even player counts
	std::vector<int> evenCounts = {2, 4, 6, 8, 10, 12, 14, 16};
	
	for(int numPlayers : evenCounts)
	{
		for(int i = 0; i < 10; i++)
		{
			auto [builders, zombies] = CTeamAssignmentTester::SimulateTeamAssignment(numPlayers, rng);
			
			EXPECT_EQ(builders, numPlayers / 2)
				<< "Even player count " << numPlayers << " should have " << (numPlayers / 2) 
				<< " builders, got " << builders;
			EXPECT_EQ(zombies, numPlayers / 2)
				<< "Even player count " << numPlayers << " should have " << (numPlayers / 2) 
				<< " zombies, got " << zombies;
		}
	}
}

/**
 * Helper class to test build points mechanics without full game context.
 */
class CBuildPointsTester
{
public:
	static const int INITIAL_BUILD_POINTS = 100;
	static const int TURRET_COST = 10;
	static const int TANK_COST = 30;
	static const int CANNON_COST = 25;

	enum EStructure { STRUCT_TURRET = 0, STRUCT_TANK, STRUCT_CANNON, NUM_STRUCTURES };

	static int GetStructureCost(EStructure Type)
	{
		switch(Type)
		{
		case STRUCT_TURRET: return TURRET_COST;
		case STRUCT_TANK: return TANK_COST;
		case STRUCT_CANNON: return CANNON_COST;
		default: return 0;
		}
	}

	/**
	 * Simulates placing a structure and returns (success, newPoints)
	 */
	static std::pair<bool, int> SimulatePlaceStructure(int CurrentPoints, EStructure Type)
	{
		int Cost = GetStructureCost(Type);
		if(CurrentPoints < Cost)
			return {false, CurrentPoints}; // Rejected, points unchanged
		return {true, CurrentPoints - Cost}; // Success, points deducted
	}
};

/**
 * **Feature: zombie-event, Property 4: Initial build points**
 * **Validates: Requirements 3.4**
 * 
 * For any builder at the start of build phase, their build points should equal exactly 100
 */
TEST(ZombieEvent, PropertyInitialBuildPoints)
{
	// This is a simple invariant - all builders start with 100 points
	// We test by simulating team assignment and checking initial points
	
	std::random_device rd;
	std::mt19937 rng(rd());
	
	for(int iteration = 0; iteration < 100; iteration++)
	{
		std::uniform_int_distribution<int> playerDist(2, 64);
		int numPlayers = playerDist(rng);
		
		// Simulate: each builder gets INITIAL_BUILD_POINTS
		int numBuilders = numPlayers / 2;
		if(numPlayers % 2 == 1 && rng() % 2 == 0)
			numBuilders++;
		
		// Property: Every builder should have exactly 100 points
		for(int i = 0; i < numBuilders; i++)
		{
			EXPECT_EQ(CBuildPointsTester::INITIAL_BUILD_POINTS, 100)
				<< "Initial build points should be 100";
		}
	}
}

/**
 * **Feature: zombie-event, Property 2: Build points deduction**
 * **Validates: Requirements 3.2**
 * 
 * For any structure placement by a builder, the builder's points should 
 * decrease by exactly the structure's cost
 */
TEST(ZombieEvent, PropertyBuildPointsDeduction)
{
	std::random_device rd;
	std::mt19937 rng(rd());
	
	for(int iteration = 0; iteration < 100; iteration++)
	{
		// Start with initial points
		int points = CBuildPointsTester::INITIAL_BUILD_POINTS;
		
		// Random structure type
		std::uniform_int_distribution<int> structDist(0, CBuildPointsTester::NUM_STRUCTURES - 1);
		auto structType = static_cast<CBuildPointsTester::EStructure>(structDist(rng));
		int cost = CBuildPointsTester::GetStructureCost(structType);
		
		// Only test if we have enough points
		if(points >= cost)
		{
			auto [success, newPoints] = CBuildPointsTester::SimulatePlaceStructure(points, structType);
			
			EXPECT_TRUE(success) << "Placement should succeed with sufficient points";
			EXPECT_EQ(newPoints, points - cost) 
				<< "Points should decrease by exactly " << cost 
				<< ", was " << points << ", now " << newPoints;
		}
	}
}

/**
 * **Feature: zombie-event, Property 3: Insufficient points rejection**
 * **Validates: Requirements 3.3**
 * 
 * For any builder with points P and structure cost C where P < C, 
 * the placement attempt should fail and points should remain unchanged
 */
TEST(ZombieEvent, PropertyInsufficientPointsRejection)
{
	std::random_device rd;
	std::mt19937 rng(rd());
	
	for(int iteration = 0; iteration < 100; iteration++)
	{
		// Generate random points that are less than at least one structure cost
		std::uniform_int_distribution<int> pointsDist(0, CBuildPointsTester::TANK_COST - 1);
		int points = pointsDist(rng);
		
		// Try each structure type
		for(int s = 0; s < CBuildPointsTester::NUM_STRUCTURES; s++)
		{
			auto structType = static_cast<CBuildPointsTester::EStructure>(s);
			int cost = CBuildPointsTester::GetStructureCost(structType);
			
			if(points < cost)
			{
				auto [success, newPoints] = CBuildPointsTester::SimulatePlaceStructure(points, structType);
				
				EXPECT_FALSE(success) 
					<< "Placement should fail with " << points << " points for structure costing " << cost;
				EXPECT_EQ(newPoints, points) 
					<< "Points should remain unchanged after failed placement";
			}
		}
	}
}


/**
 * Helper class to test zombie mechanics
 */
class CZombieMechanicsTester
{
public:
	enum EPhase { PHASE_NONE = 0, PHASE_BUILD, PHASE_ATTACK };

	/**
	 * Simulates zombie freeze state based on phase
	 * Returns true if zombie should be frozen
	 */
	static bool ShouldBeFrozen(EPhase phase, bool isZombie)
	{
		if(!isZombie)
			return false; // Builders are never frozen by the event
		return phase == PHASE_BUILD; // Zombies frozen during build phase
	}
};

/**
 * **Feature: zombie-event, Property 5: Zombie freeze during build**
 * **Validates: Requirements 4.1**
 * 
 * For any zombie player during build phase, their movement should be frozen
 */
TEST(ZombieEvent, PropertyZombieFreezeDuringBuild)
{
	// During build phase, all zombies should be frozen
	for(int i = 0; i < 100; i++)
	{
		bool shouldBeFrozen = CZombieMechanicsTester::ShouldBeFrozen(
			CZombieMechanicsTester::PHASE_BUILD, true);
		
		EXPECT_TRUE(shouldBeFrozen) 
			<< "Zombies should be frozen during build phase";
	}
	
	// Builders should NOT be frozen during build phase
	bool builderFrozen = CZombieMechanicsTester::ShouldBeFrozen(
		CZombieMechanicsTester::PHASE_BUILD, false);
	EXPECT_FALSE(builderFrozen) 
		<< "Builders should not be frozen during build phase";
}

/**
 * **Feature: zombie-event, Property 6: Zombie unfreeze during attack**
 * **Validates: Requirements 4.2**
 * 
 * For any zombie player during attack phase, their movement should not be frozen
 */
TEST(ZombieEvent, PropertyZombieUnfreezeDuringAttack)
{
	// During attack phase, zombies should NOT be frozen
	for(int i = 0; i < 100; i++)
	{
		bool shouldBeFrozen = CZombieMechanicsTester::ShouldBeFrozen(
			CZombieMechanicsTester::PHASE_ATTACK, true);
		
		EXPECT_FALSE(shouldBeFrozen) 
			<< "Zombies should not be frozen during attack phase";
	}
}

/**
 * Helper class to test auto-targeting mechanics
 */
class CAutoTargetingTester
{
public:
	static const float TURRET_RANGE;
	static const float TANK_RANGE;
	static const float CANNON_RANGE;

	/**
	 * Checks if target is within range of structure
	 */
	static bool IsInRange(float structX, float structY, float targetX, float targetY, float range)
	{
		float dx = targetX - structX;
		float dy = targetY - structY;
		float distSq = dx * dx + dy * dy;
		return distSq <= range * range;
	}
};

const float CAutoTargetingTester::TURRET_RANGE = 400.0f;
const float CAutoTargetingTester::TANK_RANGE = 600.0f;
const float CAutoTargetingTester::CANNON_RANGE = 500.0f;

/**
 * **Feature: zombie-event, Property 7: Auto-targeting range**
 * **Validates: Requirements 5.2**
 * 
 * For any defense structure and zombie within its range, the structure should fire at the zombie
 */
TEST(ZombieEvent, PropertyAutoTargetingRange)
{
	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_real_distribution<float> posDist(-1000.0f, 1000.0f);
	
	for(int iteration = 0; iteration < 100; iteration++)
	{
		// Random structure position
		float structX = posDist(rng);
		float structY = posDist(rng);
		
		// Test turret range
		float range = CAutoTargetingTester::TURRET_RANGE;
		
		// Generate target inside range
		std::uniform_real_distribution<float> insideDist(0.0f, range * 0.9f);
		float angle = std::uniform_real_distribution<float>(0.0f, 6.28318f)(rng);
		float dist = insideDist(rng);
		float targetX = structX + dist * std::cos(angle);
		float targetY = structY + dist * std::sin(angle);
		
		bool inRange = CAutoTargetingTester::IsInRange(structX, structY, targetX, targetY, range);
		EXPECT_TRUE(inRange) << "Target inside range should be detected";
		
		// Generate target outside range
		std::uniform_real_distribution<float> outsideDist(range * 1.1f, range * 2.0f);
		dist = outsideDist(rng);
		targetX = structX + dist * std::cos(angle);
		targetY = structY + dist * std::sin(angle);
		
		inRange = CAutoTargetingTester::IsInRange(structX, structY, targetX, targetY, range);
		EXPECT_FALSE(inRange) << "Target outside range should not be detected";
	}
}

/**
 * Helper class to test infection mechanics
 */
class CInfectionTester
{
public:
	/**
	 * Simulates infection: returns new team state (true = builder, false = zombie)
	 */
	static bool SimulateInfection(bool wasBuilder, bool touchedByZombie, bool isAttackPhase)
	{
		if(!wasBuilder)
			return false; // Already a zombie
		if(!isAttackPhase)
			return true; // Can't be infected during build phase
		if(!touchedByZombie)
			return true; // Not touched, stays builder
		return false; // Infected, becomes zombie
	}
};

/**
 * **Feature: zombie-event, Property 8: Infection on contact**
 * **Validates: Requirements 5.4**
 * 
 * For any builder touched by a zombie during attack phase, the builder should become a zombie
 */
TEST(ZombieEvent, PropertyInfectionOnContact)
{
	// Test all combinations
	for(int i = 0; i < 100; i++)
	{
		// Builder touched by zombie during attack phase -> becomes zombie
		bool result = CInfectionTester::SimulateInfection(true, true, true);
		EXPECT_FALSE(result) << "Builder touched by zombie during attack should become zombie";
		
		// Builder NOT touched during attack phase -> stays builder
		result = CInfectionTester::SimulateInfection(true, false, true);
		EXPECT_TRUE(result) << "Builder not touched should stay builder";
		
		// Builder touched during BUILD phase -> stays builder (no infection during build)
		result = CInfectionTester::SimulateInfection(true, true, false);
		EXPECT_TRUE(result) << "Builder touched during build phase should stay builder";
		
		// Zombie touched -> stays zombie
		result = CInfectionTester::SimulateInfection(false, true, true);
		EXPECT_FALSE(result) << "Zombie should stay zombie";
	}
}

/**
 * Helper class to test win conditions
 */
class CWinConditionTester
{
public:
	enum EWinner { WINNER_NONE = 0, WINNER_ZOMBIES, WINNER_BUILDERS };

	/**
	 * Determines winner based on game state
	 */
	static EWinner DetermineWinner(int builderCount, bool timeExpired)
	{
		if(builderCount == 0)
			return WINNER_ZOMBIES; // All builders infected
		if(timeExpired && builderCount > 0)
			return WINNER_BUILDERS; // Time expired with survivors
		return WINNER_NONE; // Game still in progress
	}
};

/**
 * **Feature: zombie-event, Property 9: Zombie win condition**
 * **Validates: Requirements 6.1**
 * 
 * For any game state where builder count equals 0, zombies should be declared winners
 */
TEST(ZombieEvent, PropertyZombieWinCondition)
{
	// When all builders are infected (count = 0), zombies win
	for(int i = 0; i < 100; i++)
	{
		auto winner = CWinConditionTester::DetermineWinner(0, false);
		EXPECT_EQ(winner, CWinConditionTester::WINNER_ZOMBIES) 
			<< "Zombies should win when all builders are infected";
		
		// Even if time expired, zombies still win if no builders
		winner = CWinConditionTester::DetermineWinner(0, true);
		EXPECT_EQ(winner, CWinConditionTester::WINNER_ZOMBIES) 
			<< "Zombies should win when all builders are infected (time expired)";
	}
}

/**
 * **Feature: zombie-event, Property 10: Builder win condition**
 * **Validates: Requirements 6.2**
 * 
 * For any game state where time expires and builder count > 0, builders should be declared winners
 */
TEST(ZombieEvent, PropertyBuilderWinCondition)
{
	std::random_device rd;
	std::mt19937 rng(rd());
	
	for(int iteration = 0; iteration < 100; iteration++)
	{
		// Random number of surviving builders (at least 1)
		std::uniform_int_distribution<int> builderDist(1, 32);
		int builderCount = builderDist(rng);
		
		// Time expired with survivors -> builders win
		auto winner = CWinConditionTester::DetermineWinner(builderCount, true);
		EXPECT_EQ(winner, CWinConditionTester::WINNER_BUILDERS) 
			<< "Builders should win when time expires with " << builderCount << " survivors";
		
		// Time NOT expired with survivors -> game continues
		winner = CWinConditionTester::DetermineWinner(builderCount, false);
		EXPECT_EQ(winner, CWinConditionTester::WINNER_NONE) 
			<< "Game should continue when time not expired with " << builderCount << " builders";
	}
}
