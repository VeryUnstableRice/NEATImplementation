#ifndef NeuralNetwork_Header
#define NeuralNetwork_Header
#include <vector>
#include <math.h>
using f_type = float;

struct sNode
{
	struct sNode_Gene* Gene;
	f_type Value;
};

struct sConnection
{
	struct sConnection_Gene* Gene;
	sNode* In;
	sNode* Out;
	bool Enabled = true;
};

class cNeuralNetwork
{
	std::vector<sNode> m_nodes;
	std::vector<sConnection> m_connections;
	class cGenome* m_parent;

	void Initialize();
	void ResetNeuralNetwork();

	void Calculate();
	std::vector<f_type> PrepareOutput();

	static f_type Sigmoid(f_type x) { return f_type(1) / (f_type(1) + f_type(exp(-x))); };
	static f_type Relu(f_type x) { return std::max(f_type(0), f_type(x)); };
	static f_type LeakyRelu(f_type x) { if (x < 0) return x * 0.1; return f_type(x); };

	friend class cGenomeMutation;
public:
	cNeuralNetwork(class cGenome* genome);
	std::vector<f_type> Evaluate(const std::vector<f_type>& inputs);
	void PrepareInputs(const std::vector<f_type>& inputs);
	void ActivateNode(sNode* LastNode);
};

#endif