#include "GenomeGenerator.h"
#include "NeuralNetwork.h"
#include <algorithm>
#include <iostream>
#include <random>

void cGenomeGenerator::SetFitnessTreshold(bool enabled, float value)
{
	m_enableFitnessTreshold = enabled;
	m_fitnessTreshhold = value;
}

cGenomeGenerator::cGenomeGenerator(int input_size, int output_size, int species_num, float(*Evaluate)(cNeuralNetwork&)):
	Evaluate(Evaluate), m_speciesNum(species_num), m_bestExemplary(input_size, output_size)
{
	Initialize_Species(input_size, output_size);
}

void cGenomeGenerator::Initialize_Species(int input_size, int output_size)
{
	cGenome initial(input_size, output_size);
	m_species.reserve(m_speciesNum);
	for (int i = 0; i < m_speciesNum; ++i)
	{
		m_species.push_back(initial);
		m_species[i].GetGenomeMutation().Mutate();
	}
}

bool cGenomeGenerator::BreedSpecies()
{
	std::sort(m_species.begin(), m_species.end(), [](const cGenome& A, const cGenome& B)
		{
			return A.GetFitness() >  B.GetFitness();
		});

	std::vector<cGenome> parent;
	parent.insert(parent.end(), m_species.begin(), m_species.begin() + 4);

	//cGenome parent1 = std::move(m_species[0]);
	//cGenome parent2 = std::move(m_species[1]);
	m_bestExemplary = parent[0];

	m_bestFitness = std::max(parent[0].GetFitness(), m_bestFitness);
	std::cout << m_bestFitness << std::endl;
	if (m_enableFitnessTreshold && m_bestFitness >= m_fitnessTreshhold)
		return true;
	std::random_device rd;
	std::mt19937 g(rd());

	m_species.clear();
	m_species.reserve(m_speciesNum);

	for (int i = 0; i < m_speciesNum; ++i)
	{
		std::shuffle(parent.begin(), parent.end(), g);

		std::uniform_int_distribution<> chance(0, 1);

		if(chance(g) <= m_crossoverRate)
			m_species.push_back(cGenome::Crossover(parent[0], parent[1]));
		else
			m_species.push_back(parent[0]);
		m_species[i].GetGenomeMutation().Mutate();
	}

	return false;
}

void cGenomeGenerator::Run(int generations)
{
	m_bestFitness = 0;
	for (int i = 0; i < generations; ++i)
	{
		for (int j = 0; j < m_speciesNum; ++j)
		{
			cNeuralNetwork network(&m_species[j]);
			m_species[j].SetFitness(Evaluate(network));
		}
		if (BreedSpecies())
			return;
	}
}
