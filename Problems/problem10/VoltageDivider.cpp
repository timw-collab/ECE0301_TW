#include "VoltageDivider.hpp"
#include <stdexcept>


VoltageDivider::VoltageDivider(double v, double res[], int num_res) : Network(num_res + 1, num_res + 1)
{
  // Base case: need at least one resistor
  if (num_res < 1)
  {
    throw std::invalid_argument("Need at least one resistor");
  }

  VoltageSource *vs = new VoltageSource();
  set_component(0, vs);
  connect_component_terminal_a(0, 0);
  connect_component_terminal_b(0, 1);
  set_component_value(0, v);

  for (int i = 0; i < num_res; i++)
  {
    Resistor *r = new Resistor();
    set_component(i + 1, r);
    connect_component_terminal_b(i + 1, i + 1);
    // Base case: last resistor returns to ground
    if (i == num_res - 1)
    {
      connect_component_terminal_a(i + 1, 0);
    }
    else
    {
      connect_component_terminal_a(i + 1, i + 2);
    }
    set_component_value(i + 1, res[i]);
  }

  double total = 0.0;
  for (int i = 0; i < num_res; i++)
  {
    total += get_resistance(i);
  }
  double current = get_component_value(0) / total;
  set_node_voltage(1, get_component_value(0));
  for (int i = 0; i < num_res - 1; i++)
  {
    set_node_voltage(i + 2, get_node_voltage(i + 1) - (current * get_resistance(i)));
  }
}


int VoltageDivider::get_num_resistors() const
{
  return get_num_components() - 1;
}


double VoltageDivider::get_source_voltage() const
{
  return get_component_voltage(0);
}

// Function edited:
double VoltageDivider::get_resistance(int idx) const
{
  // Base case: resistor index out of range
  if (idx < 0 || idx >= get_num_resistors())
  {
    throw std::out_of_range("Resistor index out of range");
  }
  return get_component_value(idx + 1);
}

\
double VoltageDivider::get_resistor_voltage(int idx) const
{
  // Base case: resistor index out of range
  if (idx < 0 || idx >= get_num_resistors())
  {
    throw std::out_of_range("Resistor index out of range");
  }
  return get_component_voltage(idx + 1);
}


double VoltageDivider::get_loop_current() const
{
  return get_component_current(1);
}


void VoltageDivider::set_source_voltage(double v)
{
  set_component_value(0, v);

  double total = 0;
  for (int i = 0; i < get_num_resistors(); i++)
  {
    total += get_resistance(i);
  }
  double current = get_component_value(0) / total;
  set_node_voltage(1, get_component_value(0));
  for (int i = 0; i < get_num_resistors() - 1; i++)
  {
    set_node_voltage(i + 2, get_node_voltage(i + 1) - current * get_resistance(i));
  }
}


void VoltageDivider::set_resistance(int idx, double r)
{
  // Base case: resistor index out of range
  if (idx < 0 || idx >= get_num_resistors())
  {
    throw std::out_of_range("Resistor index out of range");
  }
  set_component_value(idx + 1, r);

  double total = 0;
  for (int i = 0; i < get_num_resistors(); i++)
  {
    total += get_resistance(i);
  }
  double current = get_component_value(0) / total;
  set_node_voltage(1, get_component_value(0));
  for (int i = 0; i < get_num_resistors() - 1; i++)
  {
    set_node_voltage(i + 2, get_node_voltage(i + 1) - current * get_resistance(i));
  }
}
