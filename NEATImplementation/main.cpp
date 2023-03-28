// NEATImplementation.cpp : Defines the entry point for the application.
//
#include <iostream>
#include "NEAT/Genome.h"
#include "NEAT/NeuralNetwork.h"
#include "NEAT/GenomeMutation.h"
#include <unordered_map>
#include <algorithm>
#include "NEAT/GenomeGenerator.h"

std::vector<std::vector<f_type>> Inputs = { {0, 0 }, {1, 0}, {0, 1}, {1, 1} };
std::vector<std::vector<f_type>> Output = { {0}, {1}, {1}, {0} };

float Eval(cNeuralNetwork& Network)
{
	float error = 0;
	for (int j = 0; j < 4; ++j)
	{
		auto y = Network.Evaluate(Inputs[j]);
		error += (Output[j][0] - y[0])* (Output[j][0] - y[0]);
	}
	return 1 - error * 0.25;
}

int main()
{
	cGenomeGenerator generator(2, 1, 100, Eval);
	generator.SetFitnessTreshold(true, 0.9999);
	generator.Run(1000);

	cGenome exemplary = generator.GetBestExemplary();
	cNeuralNetwork neuralNetwork(&exemplary);
	
	for (int j = 0; j < 4; ++j)
	{
		auto y = neuralNetwork.Evaluate(Inputs[j]);
		std::cout << y[0] << ' ';
	}
	return 0;
}
