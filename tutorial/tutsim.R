make_agents <- function(num_agents, min_age=15, max_age=20) {
  gender <- factor(c("male","female"))
  agents <- data.frame(id=c(1:num_agents), 
                       sex=sample(gender, num_agents, replace=T), 
                       age=runif(num_agents,min_age,max_age), 
                       hiv=rgeom(num_agents, 0.9))
  return(agents)
}

agent_events <- function(agents, parameters) {
  # I don't understand the purpose of the shuffle step - it shouldn't affect the individual SC probabilities
  # Increment age
  agents$age <- agents$age + parameters$time_step
  # Infections
  prevalence <- sum(agents$hiv > 0) / nrow(agents)
  risk_infection <- parameters$force_infection * parameters$prob_new_partner * prevalence
  agents$hiv[agents$hiv==0] <- rbinom(length(agents$hiv[agents$hiv==0]),1,risk_infection)
  return(agents)
}

simulate <- function(agents, parameters) {
  time <- seq(from=parameters$start_date,
              to=parameters$start_date+parameters$num_years,
              by=parameters$time_step)
  for (t in time) {
    agents <- agent_events(agents, parameters)
    num_hiv <- sum(agents$hiv > 0)
    prevalence <- num_hiv / nrow(agents)
    print(c(t,num_hiv,prevalence))
  }
  return(agents)
}

parameters <- list(start_date=2015,
                num_years=2,
                time_step=1/365,
                prob_new_partner=0.022,
                force_infection=0.1)

agents <- make_agents(10000, min_age = 15, max_age = 20)
print(summary(agents))
agents <- simulate(agents, parameters)
print(summary(agents))

# Test how much time is taken by printing out results
system.time(simulate(agents, parameters))
simulate <- function(agents, parameters) {
  time <- seq(from=parameters$start_date,
              to=parameters$start_date+parameters$num_years,
              by=parameters$time_step)
  for (t in time) { agents <- agent_events(agents, parameters) }
  return(agents)
}
system.time(simulate(agents, parameters))