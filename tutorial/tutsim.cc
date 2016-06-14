#include <algorithm> // Some STL algorithms we will use
#include <iostream> // Input output
#include <random> // Random number generators
#include <unordered_map> // Hash table used to hold parameters
#include <vector> // Most important C++ STL data structure

// This is a Mersenee Twister random number generator. It's high quality for
// simulations. It makes sense to declare it at global/modular level because it
// would be inefficient and cumbersome to reseed locally declared
// generators. However, if you are running parallel code, then a little bit of
// extra trickery is needed, beyond the scope of this tutorial.

std::mt19937 generator;

const double YEAR = 365;

enum Sex {
  MALE = 0,
  FEMALE = 1
};

class Agent {
  // All the books will tell you it's bad to make the class variables public
  // but for our purposes I reckon it's fine. Keeps things simpler.
public:
  Sex sex;
  double age;
  /* This is the way I like to model HIV status:

     0=HIV-
     1=HIV+ primary infection
     2=HIV+ CDC stage 1
     ...
     5=HIV+ CDC stage 4
   */
  unsigned hiv;

  // This method sets the values to random numbers, but you might need
  // to replace it with something more complex, or even use a function
  // declared outside the class if you need to know the status of other agents
  void init()
  {
    // Set the sex randomly to male or female;
    {
      std::bernoulli_distribution dist(0.5);
      // This would also work fine and is a more common pattern:
      // std::uniform_int_distribution dist(0, 1);
      sex = dist(generator) == 0 ? MALE : FEMALE;
    }
    // Set the age randomly to a value between 15.0 and 20.0
    {
      std::uniform_real_distribution<double> dist(15.0, 20.0);
      age = dist(generator);
    }
    // Set the HIV status. In practice something more sophisticated than this
    // might be needed.
    {
      std::geometric_distribution<int> dist (0.9);
      // This says if it's bigger than 5 make it 5, else i.
      hiv = std::min(dist(generator), 5);
    }
  }
};

// You can also define the init function outside the class like this
void init_agent(Agent &a)
{
  // You can do this
  a.init();
  // Or you could do something like this:
  // {
  //   std::bernoulli_distribution dist(0.5);
  //   a.sex = dist(generator);
  // }
  // etc ...
}


void
initialize_agents(std::vector<Agent>& agents)
// Note the parameter declaration:
// std::vector<Agent>& agents
// This would be a mistake:
// std::vector<Agent> agents
// It is inefficient because it tells the compiler to
// make a copy of all the vector, which means making a copy of all the agents.
// And because you're modifying copies, the effect of the function would be
// to leave the vector you're passing unchanged, which is not what you want.
{
  // There are a few ways to loop through the agents. Which one you prefer
  // is often a matter of taste.

  // I think this is the most intuitive
  for (size_t i = 0; i < agents.size(); ++i)
    agents[i].init();

  // This is the iconic c++ way
  for (auto it = agents.begin(); it < agents.end(); ++it)
    it->init();

  // This is the way since c++11
  for (auto &it : agents)
    it.init();

  // Using the STL. Note this way we have to call init_agents, the function
  // outside the class
  for_each(agents.begin(), agents.end(), init_agent);
}

// Let's have a couple of events: become infected, and get older

// Expose agents to HIV and infect them. This would be replaced
// with a partner matching algorithm in a more sophisticated simulation.
// Also, we assume everyone is 100% bisexual here.

void infection_event(Agent& a,
		     const double prevalence,
		     const double prob_new_partner,
		     const double force_infection)
{
  if (a.hiv == 0) {
    double risk_infection = force_infection * prob_new_partner * prevalence;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    if (dist(generator) < risk_infection)
      a.hiv = 1;
  }
}

// Every agent has to age on each iteration of the simulation
void age_event(Agent& a, const double time_elapsed)
{
  a.age += time_elapsed;
}

// On each step of the iteration we want to do some reporting
void report(double date,  const std::vector<Agent>& agents)
{
  // Let's print the number infected
  unsigned infected = 0;
  for (auto& a: agents)
    if (a.hiv > 0)
      ++infected;
  double prevalence = (double) infected / agents.size();
  // In practise I reckon we'd want to generate this in CSV format
  std::cout << date
	    << " Num infected: " << infected << " Prevalence: " << prevalence
	    << std::endl;
}

// We need several other events too presumably, including change of infection
// status, arvs, death (unless we decide that over the short period we're
// modelling death and infection status are not so important.

// This is the simulation logic
// Note that it makes sense to keep the simulation
// parameters in a hash table which is an unordered_map in the c++ STL.

void simulate(std::vector<Agent>& agents,
	      std::unordered_map<const char *, double>& parameters)
{
  unsigned num_iterations = parameters["NUM_YEARS"] / parameters["TIME_STEP"];
  for (unsigned i = 0; i < num_iterations; ++i) {
    // So that there's no bias because of the original order of the agents
    // we shuffle them. For complex partner matching, this is vital
    shuffle(agents.begin(), agents.end(), generator);

    // For the infection event we need the prevalence
    unsigned num_infected = 0;
    for (auto & a: agents)
      if (a.hiv > 0)
	++num_infected;
    // Note that if agents die, then this becomes a touch more complicated
    double prevalence = (double) num_infected / agents.size();

    // Now iterate through the agents, doing events
    for (auto & a: agents) {
      infection_event(a, prevalence,
		      parameters["PROB_NEW_PARTNER"],
		      parameters["FORCE_INFECTION"]);
      age_event(a, parameters["TIME_STEP"]);
    }
    report(parameters["START_DATE"] + (double) i / YEAR, agents);
  }
}

void print_verbose_agent_info(std::vector<Agent>& agents)
{
  unsigned males = 0;
  std::vector<unsigned> hiv(6);
  double avg_age = 0.0;
  double youngest = agents[0].age;
  double oldest = agents[0].age;
  for (auto& a: agents) {
    ++hiv[a.hiv];
    if (a.sex == MALE) ++males;
    avg_age += a.age;
    if (a.age > oldest) oldest = a.age;
    if (a.age < youngest) youngest = a.age;
  }
  std::cout << "Males: " << males << std::endl;
  std::cout << "Youngest: " << youngest << std::endl;
  std::cout << "Oldest: " << oldest << std::endl;
  std::cout << "Average age: " << avg_age / agents.size() << std::endl;
  for (size_t i = 0; i < 6; ++i)
    std::cout << "HIV " << i << " " << hiv[i] << std::endl;
}

int main(int argc, char *argv[])
{
  // Set our parameters
  std::unordered_map<const char *, double> parameters;

  // I've just set these arbitrarily. More work needed on this
  parameters["NUM_YEARS"] = 2.0;
  parameters["TIME_STEP"] = 1.0 / YEAR; // 1 day
  parameters["START_DATE"] = 2015.0;
  // Arbitrarily chosen risk of new partner on any given day.
  // REMEMBER: If you change the TIME_STEP, you must change this.
  // Ideally we want to come up with a function of the TIME_STEP to
  // calculate this, so that we don't need to worry about it if we change
  // the TIME_STEP.
  parameters["PROB_NEW_PARTNER"] = 0.022;
  parameters["FORCE_INFECTION"] = 0.1; // 10% risk infection with HIV+ partner

  // Seed our Mersenne Twister to some arbitrarily chosen number
  generator.seed(23);
  // Note by seeding like we do above, we get the same output on every
  // execution, which is usually what we want.
  // To seed based on time, check out this code:
  // http://www.cplusplus.com/reference/random/mersenne_twister_engine/seed/

  std::vector<Agent> agents(10000); // Declare 100 agents
  initialize_agents(agents);
  // Let's get a detailed report on our demographics
  print_verbose_agent_info(agents);
  // Let's do a report before we start
  report(parameters["START_DATE"], agents);

  simulate(agents, parameters);

 // Let's check no horrendous bugs by printing demographics again
  print_verbose_agent_info(agents);
}

// Additional Notes

// To append a new agent, x, to the vector of agents:
// agents.push_back(x)

// Perhaps, a more efficient way to declare the agents, especially
// since we shuffle them on every iteration is this:
// vector<Agent *> agents
// But then you have to learn a bit about pointers, and I'm not sure the gain in
// effciency is worth the additional coding complexity.
