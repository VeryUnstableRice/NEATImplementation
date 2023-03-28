#ifndef GenomeGenerator_Header
#define GenomeGenerator_Header
#include <vector>
#include <memory>
#include "Genome.h"
#include "NeuralNetwork.h"

class cGenomeGenerator
{
	float (*Evaluate)(cNeuralNetwork&);
	std::vector<cGenome> m_species;
	int m_speciesNum; //species per generation
	float m_bestFitness;
	float m_fitnessTreshhold;
	float m_crossoverRate = 0.3;
	bool m_enableFitnessTreshold;


	cGenome m_bestExemplary;

	void Initialize_Species(int input_size, int output_size);
	bool BreedSpecies();
public:
	cGenome& GetBestExemplary() { return m_bestExemplary; };


	void SetFitnessTreshold(bool enabled, float value);
	cGenomeGenerator(int input_size, int output_size, int species_num, float(*Evaluate)(cNeuralNetwork&));
	void Run(int generations);
};

#endif