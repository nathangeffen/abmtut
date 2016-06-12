# Python version of tutsim.cc
# Implements a very basic agent based model
# Code can be used as basis for more complex models.

import numpy
import random

YEAR = 365.25
MALE = 0
FEMALE = 1

# Get the same random numbers every time we run. 23 is arbitrary
numpy.random.seed(23)
random.seed(23)


class Agent:

    def __init__(self):
        self.sex = random.randint(MALE, FEMALE)
        self.age = random.uniform(15.0, 20.0)
        self.hiv = min(numpy.random.geometric(p=0.9) - 1, 5)

    def infection_event(self, prevalence, prob_new_partner, force_infection):
        if self.hiv == 0:
            risk_infection = force_infection * prob_new_partner * prevalence
            if random.random() < risk_infection:
                self.hiv = 1

    def age_event(self, time_elapsed):
        self.age += time_elapsed


def report(date, agents):
    infected = len([a.hiv for a in agents if a.hiv > 0])
    prevalence = infected / len(agents)
    print("{0} Num infected: {1} Prevalence: {2}".
          format(date, infected, prevalence))


def simulate(agents, parameters):
    num_iterations = int(parameters['NUM_YEARS'] / parameters['TIME_STEP'])
    for i in range(num_iterations):
        random.shuffle(agents)
        prevalence = len([a.hiv for a in agents if a.hiv > 0]) / len(agents)
        for agent in agents:
            agent.infection_event(prevalence,
                                  parameters["PROB_NEW_PARTNER"],
                                  parameters["FORCE_INFECTION"])
            agent.age_event(parameters["TIME_STEP"])
        report(parameters["START_DATE"] + i / YEAR, agents)


def print_verbose_agent_info(agents):
    males = [a.sex for a in agents].count(MALE)
    avg_age = sum([a.age for a in agents]) / len(agents)
    youngest = min([a.age for a in agents])
    oldest = max([a.age for a in agents])
    hiv_counts = [[a.hiv for a in agents].count(i) for i in range(6)]
    print("DEBUG", hiv_counts)
    print("Males: ", males)
    print("Youngest: ", youngest)
    print("Oldest: ", oldest)
    print("Average age: ", avg_age)
    for i in range(6):
        print("HIV ", i, hiv_counts[i])


def main():
    parameters = {
        'NUM_YEARS': 2.0,
        'TIME_STEP': 1.0 / YEAR,
        'START_DATE': 2015.0,
        'PROB_NEW_PARTNER': 0.022,
        'FORCE_INFECTION': 0.1
    }
    agents = [Agent() for _ in range(100)]
    print_verbose_agent_info(agents)
    simulate(agents, parameters)
    print_verbose_agent_info(agents)

if __name__ == "__main__":
    main()
