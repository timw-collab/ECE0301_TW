#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <stdexcept>
#include <cmath>

#include "Node.hpp"
#include "Component.hpp"
#include "VoltageSource.hpp"
#include "CurrentSource.hpp"
#include "Resistor.hpp"

#include "Network.hpp"
#include "VoltageDivider.hpp"
#include "CurrentDivider.hpp"
#include "PracticalVsCircuit.hpp"

const double THRESHOLD = 1e-9;

bool compare_double(double val1, double val2)
{
  double diff = std::abs(val1 - val2);
  return (diff <= THRESHOLD);
}

// VoltageSource class unit tests
TEST_CASE("Network: Test VoltageSource", "[network]")
{
  VoltageSource vs;

  // test inherited behavior
  REQUIRE(vs.get_node_a() == nullptr);
  REQUIRE(vs.get_node_b() == nullptr);

  Node *node1ptr = new Node();
  Node *node2ptr = new Node();
  REQUIRE_NOTHROW(vs.set_node_a(node1ptr));
  REQUIRE_NOTHROW(vs.set_node_b(node2ptr));
  REQUIRE(vs.get_node_a() == node1ptr);
  REQUIRE(vs.get_node_b() == node2ptr);
  REQUIRE(compare_double(vs.get_voltage(), 0));

  // test source voltage, should start as 0 from Component default
  double voltage = 3.3;
  REQUIRE(compare_double(vs.get_voltage(), 0));
  vs.set_value(voltage);
  // source voltage should be independent of node voltages
  REQUIRE(compare_double(vs.get_voltage(), voltage));

  // construct with pointers, test that source voltage is separate from terminal voltages
  double v1 = 5.2, v2 = 1.3;
  node1ptr->set_voltage(v1);
  node2ptr->set_voltage(v2);
  VoltageSource vs2(node1ptr, node2ptr);
  REQUIRE(compare_double(vs2.get_voltage(), 0));
  vs2.set_value(-voltage);
  REQUIRE(compare_double(vs2.get_voltage(), -voltage));

  delete node1ptr;
  delete node2ptr;
}

// CurrentSource class unit tests
TEST_CASE("Network: Test CurrentSource", "[network]")
{
  // test inherited behavior
  CurrentSource cs;

  REQUIRE(cs.get_node_a() == nullptr);
  REQUIRE(cs.get_node_b() == nullptr);
  REQUIRE_THROWS_AS(cs.get_voltage(), std::logic_error);

  Node *node1ptr = new Node();
  Node *node2ptr = new Node();
  REQUIRE_NOTHROW(cs.set_node_a(node1ptr));
  REQUIRE_NOTHROW(cs.set_node_b(node2ptr));
  REQUIRE(cs.get_node_a() == node1ptr);
  REQUIRE(cs.get_node_b() == node2ptr);
  REQUIRE(compare_double(cs.get_voltage(), 0));

  // test source current, should start as 0 from Component default
  double current = 3.3;
  REQUIRE(compare_double(cs.get_current(), 0));
  cs.set_value(current);
  REQUIRE(compare_double(cs.get_current(), current));

  // construct with pointers, test current and voltage
  double v1 = 5.2, v2 = 1.3;
  node1ptr->set_voltage(v1);
  node2ptr->set_voltage(v2);
  CurrentSource cs2(node1ptr, node2ptr);
  REQUIRE(compare_double(cs2.get_current(), 0));
  cs2.set_value(-current);
  REQUIRE(compare_double(cs2.get_voltage(), v2 - v1));
  REQUIRE(compare_double(cs2.get_current(), -current));

  delete node1ptr;
  delete node2ptr;
}

// Resistor class unit tests
TEST_CASE("Network: Test Resistor", "[network]")
{
  // test inheritance
  Resistor r;

  REQUIRE(r.get_node_a() == nullptr);
  REQUIRE(r.get_node_b() == nullptr);
  REQUIRE_THROWS_AS(r.get_voltage(), std::logic_error);

  Node *node1ptr = new Node();
  Node *node2ptr = new Node();
  REQUIRE_NOTHROW(r.set_node_a(node1ptr));
  REQUIRE_NOTHROW(r.set_node_b(node2ptr));
  REQUIRE(r.get_node_a() == node1ptr);
  REQUIRE(r.get_node_b() == node2ptr);
  REQUIRE(compare_double(r.get_voltage(), 0));

  // test resistance, should start as 0 from Component default
  double resistance = 590;
  REQUIRE(compare_double(r.get_value(), 0));
  r.set_value(0); // 0 resistance is allowed
  r.set_value(resistance);
  REQUIRE(compare_double(r.get_value(), resistance));

  // test negative resistance
  REQUIRE_THROWS_AS(r.set_value(-resistance), std::invalid_argument);

  // test current
  double v1 = 5.2, v2 = 1.3;
  node1ptr->set_voltage(v1);
  node2ptr->set_voltage(v2);
  REQUIRE(compare_double(r.get_current(), (v2 - v1) / resistance));

  // test get_current throws
  r.set_node_a(nullptr);
  // node a nullptr
  REQUIRE_THROWS_AS(r.get_current(), std::logic_error);
  r.set_node_b(nullptr);
  // both nodes nullptr
  REQUIRE_THROWS_AS(r.get_current(), std::logic_error);
  r.set_node_a(node1ptr);
  // node b still nullptr
  REQUIRE_THROWS_AS(r.get_current(), std::logic_error);
  r.set_node_b(node2ptr);
  // valid
  REQUIRE(compare_double(r.get_current(), (v2 - v1) / resistance));
  // div by 0
  r.set_value(0);
  REQUIRE_THROWS_AS(r.get_current(), std::logic_error);

  delete node1ptr;
  delete node2ptr;
}

// General unit tests of overriding and dynamic binding
TEST_CASE("Network: Test overriding and dynamic binding", "[network]")
{
  Component c;
  Resistor r;
  Component *c_ptr = new Resistor();

  REQUIRE_NOTHROW(c.set_value(-1));
  REQUIRE_THROWS_AS(r.set_value(-1), std::invalid_argument);
  REQUIRE_THROWS_AS(c_ptr->set_value(-1), std::invalid_argument);

  c.set_value(0);
  r.set_value(0);
  c_ptr->set_value(0);
  REQUIRE_NOTHROW(c.get_current());
  REQUIRE_THROWS_AS(r.get_current(), std::logic_error);
  REQUIRE_THROWS_AS(c_ptr->get_current(), std::logic_error);

  double val = 0.1;
  Component c_base(val);
  VoltageSource vs(val);
  CurrentSource cs(val);
  Component *c_ptr1 = new VoltageSource(val);
  Component *c_ptr2 = new CurrentSource(val);

  REQUIRE(compare_double(c_base.get_current(), 0.0));
  REQUIRE(compare_double(vs.get_current(), 0.0));
  REQUIRE(compare_double(cs.get_current(), val));
  REQUIRE(compare_double(c_ptr1->get_current(), 0.0));
  REQUIRE(compare_double(c_ptr2->get_current(), val));

  REQUIRE_THROWS_AS(c_base.get_voltage(), std::logic_error);
  REQUIRE(compare_double(vs.get_voltage(), val));
  REQUIRE_THROWS_AS(cs.get_voltage(), std::logic_error);
  REQUIRE(compare_double(c_ptr1->get_voltage(), val));
  REQUIRE_THROWS_AS(c_ptr2->get_voltage(), std::logic_error);

  delete c_ptr;
  delete c_ptr1;
  delete c_ptr2;
}

// Write your own test cases here.

// Uncomment these partial test cases when you are ready to test the dividers.

TEST_CASE("Network: Test VoltageDivider", "[network]")
{
  double voltage = 5.0;
  const int num = 2;
  double res[num] = {200, 400};
  VoltageDivider div(voltage, res, num);

  // TODO
}

TEST_CASE("Network: Test CurrentDivider", "[network]")
{
  double current = 0.006;
  const int num = 2;
  double res[num] = {15000, 75000};
  CurrentDivider div(current, res, num);

  // TODO
}

//Section 4: CurrentDivider
TEST_CASE("Network: Test CurrentDivider", "[network]")
{
  double current = 0.006;
  const int num = 2;
  double res[num] = {15000, 75000};
  CurrentDivider div(current, res, num);

  // structure: always 2 nodes, num + 1 components
  REQUIRE(div.get_num_resistors() == 2);
  REQUIRE(div.get_num_nodes() == 2);
  REQUIRE(div.get_num_components() == 3);

  // values after construction
  REQUIRE(compare_double(div.get_source_current(), 0.006));
  REQUIRE(compare_double(div.get_resistance(0), 15000));
  REQUIRE(compare_double(div.get_resistance(1), 75000));

  // R_eq = 12500, V = 0.006 * 12500 = 75
  REQUIRE(compare_double(div.get_resistor_voltage(), 75));
  REQUIRE(compare_double(div.get_node_voltage(0), 0));
  REQUIRE(compare_double(div.get_node_voltage(1), 75));
  REQUIRE(compare_double(div.get_resistor_current(0), 0.005));
  REQUIRE(compare_double(div.get_resistor_current(1), 0.001));

  // KCL: resistor currents add up to the source current
  REQUIRE(compare_double(div.get_resistor_current(0) + div.get_resistor_current(1), div.get_source_current()));

  // set_source_current: V = 0.012 * 12500 = 150
  div.set_source_current(0.012);
  REQUIRE(compare_double(div.get_source_current(), 0.012));
  REQUIRE(compare_double(div.get_resistor_voltage(), 150));
  REQUIRE(compare_double(div.get_resistor_current(0), 0.01));
  REQUIRE(compare_double(div.get_resistor_current(1), 0.002));

  // set_resistance: both 15000, R_eq = 7500, V = 90
  div.set_resistance(1, 15000);
  REQUIRE(compare_double(div.get_resistance(1), 15000));
  REQUIRE(compare_double(div.get_resistor_voltage(), 90));
  REQUIRE(compare_double(div.get_resistor_current(0), 0.006));
  REQUIRE(compare_double(div.get_resistor_current(1), 0.006));

  // index exceptions
  REQUIRE_THROWS_AS(div.get_resistance(-1), std::out_of_range);
  REQUIRE_THROWS_AS(div.get_resistance(2), std::out_of_range);
  REQUIRE_THROWS_AS(div.get_resistor_current(-1), std::out_of_range);
  REQUIRE_THROWS_AS(div.get_resistor_current(2), std::out_of_range);
  REQUIRE_THROWS_AS(div.set_resistance(-1, 100), std::out_of_range);
  REQUIRE_THROWS_AS(div.set_resistance(2, 100), std::out_of_range);

  // negative resistance
  REQUIRE_THROWS_AS(div.set_resistance(0, -100), std::invalid_argument);

  // constructor exceptions
  REQUIRE_THROWS_AS(CurrentDivider(current, res, 0), std::invalid_argument);
  REQUIRE_THROWS_AS(CurrentDivider(current, res, -1), std::invalid_argument);
  double bad_res[num] = {-15000, 75000};
  REQUIRE_THROWS_AS(CurrentDivider(current, bad_res, num), std::invalid_argument);
}

TEST_CASE("Network: Test CurrentDivider with three resistors", "[network]")
{
  const int num = 3;
  double res[num] = {100, 200, 200};
  CurrentDivider div(0.1, res, num);

  // G = 0.01 + 0.005 + 0.005 = 0.02, V = 0.1 / 0.02 = 5
  REQUIRE(compare_double(div.get_resistor_voltage(), 5));
  REQUIRE(compare_double(div.get_resistor_current(0), 0.05));
  REQUIRE(compare_double(div.get_resistor_current(1), 0.025));
  REQUIRE(compare_double(div.get_resistor_current(2), 0.025));

  // every resistor has the same voltage in parallel
  REQUIRE(compare_double(div.get_component_voltage(1), div.get_component_voltage(3)));
}
























