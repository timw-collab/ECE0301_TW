#include "Resistor.hpp"
#include <stdexcept>

void Resistor::set_value(double r)
{
  // TODO
  if (r < 0)
  {
    throw std::invalid_argument("Resistance cannot be negative");
  }
  Component::set_value(r);
}

double Resistor::get_current() const
{
  // TODO
  if (get_value() == 0)
  {
    throw std::logic_error("Resistance is 0");
  }
  return get_voltage() / get_value();
}
