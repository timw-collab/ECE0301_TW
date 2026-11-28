#include "PracticalVsCircuit.hpp"
#include <vector>
#include <cmath>


PracticalVsCircuit::PracticalVsCircuit(double vs, double rs, double rl) : VoltageDivider(vs, std::vector<double>(2, 1).data(), 2)
{
  set_resistance(0, rs);
  set_resistance(1, rl);
}

double PracticalVsCircuit::get_source_voltage() const
{
  return VoltageDivider::get_source_voltage() - get_resistor_voltage(0);
}


void PracticalVsCircuit::set_load(double rl)
{
  set_resistance(1, rl);
}


double PracticalVsCircuit::get_load_power() const
{
  return get_resistor_voltage(1) * get_resistor_voltage(1) / get_resistance(1);
}


bool PracticalVsCircuit::maximum_power_transfer(double rl)
{
  set_load(rl);
  double vs = VoltageDivider::get_source_voltage();
  double p_max = vs * vs / (4 * get_resistance(0));
  
  return std::abs(get_load_power() - p_max) <= 1e-9;
}
