#include "CurrentDivider.hpp"
#include <stdexcept>


CurrentDivider::CurrentDivider(double c, double res[], int num_res) : Network(2, num_res + 1)
{
  // Base case: need at least one resistor
  if (num_res < 1)
  {
    throw std::invalid_argument("Need at least one resistor");
  }

  CurrentSource *cs = new CurrentSource();
  set_component(0, cs);
  connect_component_terminal_a(0, 0);
  connect_component_terminal_b(0, 1);
  set_component_value(0, c);

  for (int i = 0; i < num_res; i++)
  {
    Resistor *r = new Resistor();
    set_component(i + 1, r);
    connect_component_terminal_a(i + 1, 0);
    connect_component_terminal_b(i + 1, 1);
    set_component_value(i + 1, res[i]);
  }

  double conductance = 0;
  bool shorted = false;
  for (int i = 0; i < num_res; i++)
  {
    // Base case: a 0-ohm resistor shorts Node 1 to ground
    if (get_resistance(i) == 0)
    {
      shorted = true;
    }
    else
    {
      conductance = conductance + 1 / get_resistance(i);
    }
  }
  if (shorted)
  {
    set_node_voltage(1, 0);
  }
  else
  {
    set_node_voltage(1, get_source_current() / conductance);
  }
}


int CurrentDivider::get_num_resistors() const
{
  return get_num_components() - 1;
}


double CurrentDivider::get_source_current() const
{
  return get_component_current(0);
}


double CurrentDivider::get_resistance(int idx) const
{
  // Base case: resistor index out of range
  if (idx < 0 || idx >= get_num_resistors())
  {
    throw std::out_of_range("Resistor index out of range");
  }
  return get_component_value(idx + 1);
}


double CurrentDivider::get_resistor_voltage() const
{
  return get_component_voltage(1);
}


double CurrentDivider::get_resistor_current(int idx) const
{
  // Base case: resistor index out of range
  if (idx < 0 || idx >= get_num_resistors())
  {
    throw std::out_of_range("Resistor index out of range");
  }
  return get_component_current(idx + 1);
}


void CurrentDivider::set_source_current(double c)
{
  set_component_value(0, c);

  double conductance = 0;
  bool shorted = false;
  for (int i = 0; i < get_num_resistors(); i++)
  {
    // Base case: a 0-ohm resistor shorts Node 1 to ground
    if (get_resistance(i) == 0)
    {
      shorted = true;
    }
    else
    {
      conductance = conductance + 1 / get_resistance(i);
    }
  }
  if (shorted)
  {
    set_node_voltage(1, 0);
  }
  else
  {
    set_node_voltage(1, get_source_current() / conductance);
  }
}


void CurrentDivider::set_resistance(int idx, double r)
{
  // Base case: resistor index out of range
  if (idx < 0 || idx >= get_num_resistors())
  {
    throw std::out_of_range("Resistor index out of range");
  }
  set_component_value(idx + 1, r);

  double conductance = 0;
  bool shorted = false;
  for (int i = 0; i < get_num_resistors(); i++)
  {
    // Base case: a 0-ohm resistor shorts Node 1 to ground
    if (get_resistance(i) == 0)
    {
      shorted = true;
    }
    else
    {
      conductance = conductance + 1 / get_resistance(i);
    }
  }
  if (shorted)
  {
    set_node_voltage(1, 0);
  }
  else
  {
    set_node_voltage(1, get_source_current() / conductance);
  }
}
