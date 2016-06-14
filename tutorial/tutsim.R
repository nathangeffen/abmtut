YEAR = 365
MALE = 0
FEMALE = 1

make_agents <- function(num_agents) {
  id <- c(1:100)
  sex <- sample(MALE:FEMALE, num_agents, replace=T)
  age <- runif(num_agents,15,20)
  hiv = rgeom(num_agents, 0.9)
  agents <- data.frame(id=id, sex=sex, age=age, hiv=hiv)
  agents
}

agent_events <- function(agents, parameters) {
  num_agents = nrow(agents)
  # Shuffle
  agents = agents[sample(num_agents, num_agents), ]
  # Increment age
  agents$age = agents$age + parameters$TIME_STEP
  # Infections
  prevalence = sum(agents$hiv > 0) / num_agents
  risk_infection = parameters$FORCE_INFECTION * parameters$PROB_NEW_PARTNER * prevalence
  # This is ugly? Sets agents who are HIV- to HIV+ if random number generator
  # produces value < risk_infection.
  agents$hiv[agents$hiv==0] = runif(length(agents$hiv[agents$hiv==0])) < risk_infection
  agents
}

simulate <- function(agents, parameters) {
  num_iterations = parameters$NUM_YEARS / parameters$TIME_STEP
  for (i in 1:num_iterations) {
      agents = agent_events(agents, parameters)
      current_date = parameters$START_DATE + i * parameters$TIME_STEP
      num_hiv = sum(agents$hiv > 0)
      prevalence = num_hiv / nrow(agents)
      output = c(current_date, num_hiv, prevalence)
      print(output)
  }
  agents
}

NUM_YEARS = 2.0
TIME_STEP = 1.0 / YEAR
START_DATE = 2015.0
PROB_NEW_PARTNER = 0.022
FORCE_INFECTION = 0.1
parameters = data.frame(NUM_YEARS=NUM_YEARS,
                        TIME_STEP=TIME_STEP,
                        START_DATE=START_DATE,
                        PROB_NEW_PARTNER=PROB_NEW_PARTNER,
                        FORCE_INFECTION=FORCE_INFECTION)
agents = make_agents(10000)
print(summary(agents))
agents = simulate(agents, parameters)
print(summary(agents))
