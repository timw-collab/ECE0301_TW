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

//Section 3: VoltageDivider
TEST_CASE("Network: Test VoltageDivider", "[network]")
{
  double voltage = 5.0;
  const int num = 2;
  double res[num] = {200, 400};
  VoltageDivider div(voltage, res, num);

  // structure: num + 1 nodes, num + 1 components
  REQUIRE(div.get_num_resistors() == 2);
  REQUIRE(div.get_num_nodes() == 3);
  REQUIRE(div.get_num_components() == 3);

  // values after construction
  REQUIRE(compare_double(div.get_source_voltage(), 5.0));
  REQUIRE(compare_double(div.get_resistance(0), 200));
  REQUIRE(compare_double(div.get_resistance(1), 400));

  // I = 5 / 600, V0 = I * 200, V1 = I * 400
  REQUIRE(compare_double(div.get_loop_current(), 5.0 / 600));
  REQUIRE(compare_double(div.get_resistor_voltage(0), 5.0 / 3));
  REQUIRE(compare_double(div.get_resistor_voltage(1), 10.0 / 3));

  // node voltages
  REQUIRE(compare_double(div.get_node_voltage(0), 0));
  REQUIRE(compare_double(div.get_node_voltage(1), 5.0));
  REQUIRE(compare_double(div.get_node_voltage(2), 10.0 / 3));

  // same current through every resistor
  REQUIRE(compare_double(div.get_component_current(1), div.get_component_current(2)));

  // set_source_voltage: I = 12 / 600 = 0.02
  div.set_source_voltage(12);
  REQUIRE(compare_double(div.get_source_voltage(), 12));
  REQUIRE(compare_double(div.get_loop_current(), 0.02));
  REQUIRE(compare_double(div.get_resistor_voltage(0), 4));
  REQUIRE(compare_double(div.get_resistor_voltage(1), 8));
  REQUIRE(compare_double(div.get_node_voltage(2), 8));

  // set_resistance: R1 = 200, I = 12 / 400 = 0.03
  div.set_resistance(1, 200);
  REQUIRE(compare_double(div.get_resistance(1), 200));
  REQUIRE(compare_double(div.get_loop_current(), 0.03));
  REQUIRE(compare_double(div.get_resistor_voltage(0), 6));
  REQUIRE(compare_double(div.get_resistor_voltage(1), 6));
  REQUIRE(compare_double(div.get_node_voltage(2), 6));

  // index exceptions
  REQUIRE_THROWS_AS(div.get_resistance(-1), std::out_of_range);
  REQUIRE_THROWS_AS(div.get_resistance(2), std::out_of_range);
  REQUIRE_THROWS_AS(div.get_resistor_voltage(-1), std::out_of_range);
  REQUIRE_THROWS_AS(div.get_resistor_voltage(2), std::out_of_range);
  REQUIRE_THROWS_AS(div.set_resistance(-1, 100), std::out_of_range);
  REQUIRE_THROWS_AS(div.set_resistance(2, 100), std::out_of_range);

  // negative resistance
  REQUIRE_THROWS_AS(div.set_resistance(0, -100), std::invalid_argument);

  // constructor exceptions
  REQUIRE_THROWS_AS(VoltageDivider(voltage, res, 0), std::invalid_argument);
  REQUIRE_THROWS_AS(VoltageDivider(voltage, res, -1), std::invalid_argument);
  double bad_res[num] = {200, -400};
  REQUIRE_THROWS_AS(VoltageDivider(voltage, bad_res, num), std::invalid_argument);
}

TEST_CASE("Network: Test VoltageDivider with three resistors", "[network]")
{
  const int num = 3;
  double res[num] = {100, 200, 300};
  VoltageDivider div(9, res, num);

  // I = 9 / 600 = 0.015, drops 1.5, 3, 4.5
  REQUIRE(div.get_num_nodes() == 4);
  REQUIRE(compare_double(div.get_loop_current(), 0.015));
  REQUIRE(compare_double(div.get_resistor_voltage(0), 1.5));
  REQUIRE(compare_double(div.get_resistor_voltage(1), 3));
  REQUIRE(compare_double(div.get_resistor_voltage(2), 4.5));
  REQUIRE(compare_double(div.get_node_voltage(1), 9));
  REQUIRE(compare_double(div.get_node_voltage(2), 7.5));
  REQUIRE(compare_double(div.get_node_voltage(3), 4.5));

  // a single resistor gets the full source voltage
  double one_res[1] = {1000};
  VoltageDivider div1(3.3, one_res, 1);
  REQUIRE(div1.get_num_nodes() == 2);
  REQUIRE(compare_double(div1.get_resistor_voltage(0), 3.3));
  REQUIRE(compare_double(div1.get_loop_current(), 0.0033));
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

//Section 5: PracticalVsCircuit 
TEST_CASE("Network: Test PracticalVsCircuit", "[network]")
{
  // VS = 10, RS = 50, RL = 150: I = 0.05, VL = 7.5
  PracticalVsCircuit pvs(10, 50, 150);
  REQUIRE(pvs.get_num_resistors() == 2);
  REQUIRE(compare_double(pvs.get_resistance(0), 50));
  REQUIRE(compare_double(pvs.get_resistance(1), 150));
  REQUIRE(compare_double(pvs.get_source_voltage(), 7.5));
  REQUIRE(compare_double(pvs.VoltageDivider::get_source_voltage(), 10));
  REQUIRE(compare_double(pvs.get_load_power(), 0.375));

  // set_load: RL = 450, I = 0.02, VL = 9, PL = 0.18
  pvs.set_load(450);
  REQUIRE(compare_double(pvs.get_resistance(1), 450));
  REQUIRE(compare_double(pvs.get_source_voltage(), 9));
  REQUIRE(compare_double(pvs.get_load_power(), 0.18));

  // matched load: RL = RS gives Pmax = 100 / 200 = 0.5
  REQUIRE(pvs.maximum_power_transfer(50));
  REQUIRE(compare_double(pvs.get_resistance(1), 50));
  REQUIRE(compare_double(pvs.get_source_voltage(), 5));
  REQUIRE(compare_double(pvs.get_load_power(), 0.5));

  // unmatched load: still changes the circuit, VL = 10 * 100 / 150
  REQUIRE_FALSE(pvs.maximum_power_transfer(100));
  REQUIRE(compare_double(pvs.get_resistance(1), 100));
  REQUIRE(compare_double(pvs.get_source_voltage(), 20.0 / 3));
  REQUIRE(pvs.get_load_power() < 0.5);
  REQUIRE_FALSE(pvs.maximum_power_transfer(10));

  // exceptions
  REQUIRE_THROWS_AS(PracticalVsCircuit(10, -50, 150), std::invalid_argument);
  REQUIRE_THROWS_AS(PracticalVsCircuit(10, 50, -150), std::invalid_argument);
  REQUIRE_THROWS_AS(pvs.set_load(-1), std::invalid_argument);
  REQUIRE_THROWS_AS(pvs.maximum_power_transfer(-1), std::invalid_argument);
}

TEST_CASE("Network: Test PracticalVsCircuit dynamic and static binding", "[network]")
{
  // VoltageDivider pointer to a PracticalVsCircuit
  VoltageDivider *vd_ptr = new PracticalVsCircuit(10, 50, 150);
  // dynamic binding: the PracticalVsCircuit version runs (terminal voltage)
  REQUIRE(compare_double((*vd_ptr).get_source_voltage(), 7.5));
  // qualified call is bound at compile time: the VoltageDivider version runs (ideal voltage)
  REQUIRE(compare_double((*vd_ptr).VoltageDivider::get_source_voltage(), 10));

  // Network pointer to a PracticalVsCircuit
  Network *net_ptr = new PracticalVsCircuit(10, 50, 150);
  REQUIRE(compare_double((*net_ptr).get_node_voltage(0), 0));
  REQUIRE(compare_double((*net_ptr).get_node_voltage(1), 10));
  REQUIRE(compare_double((*net_ptr).get_node_voltage(2), 7.5));

  // safe because ~Network is virtual
  delete vd_ptr;
  delete net_ptr;
}

TEST_CASE("Network: Test Resistor through reference and pointer", "[network]")
{
  Resistor r;
  Component &c_ref = r;
  Component *c_ptr = &r;

  // set_value is virtual, so the Resistor version runs through both
  REQUIRE_THROWS_AS(c_ref.set_value(-1), std::invalid_argument);
  REQUIRE_THROWS_AS((*c_ptr).set_value(-1), std::invalid_argument);
  REQUIRE_NOTHROW(c_ref.set_value(100));
  REQUIRE(compare_double((*c_ptr).get_value(), 100));
}
