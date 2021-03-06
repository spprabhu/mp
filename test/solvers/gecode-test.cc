/*
 Gecode solver tests.

 Copyright (C) 2012 AMPL Optimization Inc

 Permission to use, copy, modify, and distribute this software and its
 documentation for any purpose and without fee is hereby granted,
 provided that the above copyright notice appear in all copies and that
 both that the copyright notice and this permission notice and warranty
 disclaimer appear in supporting documentation.

 The author and AMPL Optimization Inc disclaim all warranties with
 regard to this software, including all implied warranties of
 merchantability and fitness.  In no event shall the author be liable
 for any special, indirect or consequential damages or any damages
 whatsoever resulting from loss of use, data or profits, whether in an
 action of contract, negligence or other tortious action, arising out
 of or in connection with the use or performance of this software.

 Author: Victor Zverovich
 */

#include <algorithm>
#include <memory>
#include <string>

#include "gecode/gecode.h"
#include "feature.h"
#include "../util.h"

typedef mp::GecodeSolver Solver;
enum {FEATURES = 0};

#include "nl-solver-test.h"

using std::string;
using Gecode::IntVarBranch;
using mp::InvalidOptionValue;
using mp::Problem;

// ----------------------------------------------------------------------------
// Solver tests

TEST_F(NLSolverTest, FloorSqrt) {
  auto factory = MakeUnaryExprFactory(
        mp::expr::FLOOR, MakeUnaryExprFactory(
          mp::expr::SQRT, VariableFactory(1)));
  EXPECT_EQ(6, Eval(factory, 42));
}

// ----------------------------------------------------------------------------
// Option tests

TEST_F(NLSolverTest, ADOption) {
  EXPECT_EQ(Gecode::Search::Options().a_d, solver_.options().a_d);
  solver_.SetIntOption("a_d", 42);
  EXPECT_EQ(42u, solver_.options().a_d);
  EXPECT_EQ(42, solver_.GetIntOption("a_d"));
  EXPECT_THROW(solver_.SetIntOption("a_d", -1), InvalidOptionValue);
}

TEST_F(NLSolverTest, CDOption) {
  EXPECT_EQ(Gecode::Search::Options().c_d, solver_.options().c_d);
  solver_.SetIntOption("c_d", 42);
  EXPECT_EQ(42u, solver_.options().c_d);
  EXPECT_EQ(42, solver_.GetIntOption("c_d"));
  EXPECT_THROW(solver_.SetIntOption("c_d", -1), InvalidOptionValue);
}

TEST_F(NLSolverTest, FailLimitOption) {
  ProblemBuilder pb(solver_);
  MakeTSP(pb);
  solver_.SetIntOption("faillimit", 10);
  TestSolutionHandler sh;
  solver_.Solve(pb, sh);
  EXPECT_EQ(402, sh.status());
  EXPECT_TRUE(sh.message().find(" 11 fails") != string::npos);
  EXPECT_EQ(10, solver_.GetIntOption("faillimit"));
  EXPECT_THROW(solver_.SetIntOption("faillimit", -1), InvalidOptionValue);
}

TEST_F(NLSolverTest, NodeLimitOption) {
  ProblemBuilder pb(solver_);
  MakeTSP(pb);
  solver_.SetIntOption("nodelimit", 10);
  TestSolutionHandler sh;
  solver_.Solve(pb, sh);
  EXPECT_EQ(401, sh.status());
  EXPECT_TRUE(sh.message().find("11 nodes") != string::npos);
  EXPECT_EQ(10, solver_.GetIntOption("nodelimit"));
  EXPECT_THROW(solver_.SetIntOption("nodelimit", -1), InvalidOptionValue);
}

TEST_F(NLSolverTest, TimeLimitOption) {
  ProblemBuilder pb(solver_);
  MakeTSP(pb);
  solver_.SetDblOption("timelimit", 0.1);
  EXPECT_EQ(400, Solve(pb).solve_code());
  EXPECT_EQ(0.1, solver_.GetDblOption("timelimit"));
  EXPECT_THROW(solver_.SetDblOption("timelimit", -1), InvalidOptionValue);
}

TEST_F(NLSolverTest, ThreadsOption) {
  EXPECT_EQ(Gecode::Search::Options().threads, solver_.options().threads);
  solver_.SetDblOption("threads", 0.5);
  EXPECT_EQ(0.5, solver_.GetDblOption("threads"));
  EXPECT_EQ(0.5, solver_.options().threads);
  solver_.SetDblOption("threads", -10);
  EXPECT_EQ(-10.0, solver_.options().threads);
}

template <typename T>
struct OptionValue {
  const char *name;
  T value;
};

const OptionValue<Gecode::IntConLevel> INT_CON_LEVELS[] = {
    {"val", Gecode::ICL_VAL},
    {"bnd", Gecode::ICL_BND},
    {"dom", Gecode::ICL_DOM},
    {"def", Gecode::ICL_DEF},
    {0,     Gecode::ICL_VAL}
};

TEST_F(NLSolverTest, IntConLevelOption) {
  EXPECT_EQ(Gecode::ICL_DEF, solver_.icl());
  unsigned count = 0;
  for (const OptionValue<Gecode::IntConLevel>
      *p = INT_CON_LEVELS; p->name; ++p, ++count) {
    solver_.SetStrOption("icl", p->name);
    EXPECT_EQ(p->name, solver_.GetStrOption("icl"));
    EXPECT_EQ(p->value, solver_.icl());
  }
  EXPECT_EQ(4u, count);
}

const OptionValue<Gecode::IntValBranch> VAL_BRANCHINGS[] = {
    {"min",        Gecode::INT_VAL_MIN()},
    {"med",        Gecode::INT_VAL_MED()},
    {"max",        Gecode::INT_VAL_MAX()},
    {"rnd",        Gecode::INT_VAL_RND(Gecode::Rnd(0))},
    {"split_min",  Gecode::INT_VAL_SPLIT_MIN()},
    {"split_max",  Gecode::INT_VAL_SPLIT_MAX()},
    {"range_min",  Gecode::INT_VAL_RANGE_MIN()},
    {"range_max",  Gecode::INT_VAL_RANGE_MAX()},
    {"values_min", Gecode::INT_VALUES_MIN()},
    {"values_max", Gecode::INT_VALUES_MAX()},
    {0,            Gecode::INT_VAL_MIN()}
};

TEST_F(NLSolverTest, ValBranchingOption) {
  EXPECT_EQ(Gecode::INT_VAL_MIN().select(), solver_.val_branching().select());
  unsigned count = 0;
  for (const OptionValue<Gecode::IntValBranch>
      *p = VAL_BRANCHINGS; p->name; ++p, ++count) {
    solver_.SetStrOption("val_branching", p->name);
    EXPECT_EQ(p->name, solver_.GetStrOption("val_branching"));
    EXPECT_EQ(p->value.select(), solver_.val_branching().select());
  }
  EXPECT_EQ(10u, count);
}

const OptionValue<IntVarBranch::Select> VAR_BRANCHINGS[] = {
    {"none",              IntVarBranch::SEL_NONE},
    {"rnd",               IntVarBranch::SEL_RND},
    {"degree_min",        IntVarBranch::SEL_DEGREE_MIN},
    {"degree_max",        IntVarBranch::SEL_DEGREE_MAX},
    {"afc_min",           IntVarBranch::SEL_AFC_MIN},
    {"afc_max",           IntVarBranch::SEL_AFC_MAX},
    {"activity_min",      IntVarBranch::SEL_ACTIVITY_MIN},
    {"activity_max",      IntVarBranch::SEL_ACTIVITY_MAX},
    {"min_min",           IntVarBranch::SEL_MIN_MIN},
    {"min_max",           IntVarBranch::SEL_MIN_MAX},
    {"max_min",           IntVarBranch::SEL_MAX_MIN},
    {"max_max",           IntVarBranch::SEL_MAX_MAX},
    {"size_min",          IntVarBranch::SEL_SIZE_MIN},
    {"size_max",          IntVarBranch::SEL_SIZE_MAX},
    {"degree_size_min",   IntVarBranch::SEL_DEGREE_SIZE_MIN},
    {"degree_size_max",   IntVarBranch::SEL_DEGREE_SIZE_MAX},
    {"afc_size_min",      IntVarBranch::SEL_AFC_SIZE_MIN},
    {"afc_size_max",      IntVarBranch::SEL_AFC_SIZE_MAX},
    {"activity_size_min", IntVarBranch::SEL_ACTIVITY_SIZE_MIN},
    {"activity_size_max", IntVarBranch::SEL_ACTIVITY_SIZE_MAX},
    {"regret_min_min",    IntVarBranch::SEL_REGRET_MIN_MIN},
    {"regret_min_max",    IntVarBranch::SEL_REGRET_MIN_MAX},
    {"regret_max_min",    IntVarBranch::SEL_REGRET_MAX_MIN},
    {"regret_max_max",    IntVarBranch::SEL_REGRET_MAX_MAX},
    {0,                   IntVarBranch::SEL_NONE}
};

TEST_F(NLSolverTest, VarBranchingOption) {
  EXPECT_EQ(IntVarBranch::SEL_SIZE_MIN, solver_.var_branching());
  unsigned count = 0;
  for (const OptionValue<IntVarBranch::Select>
      *p = VAR_BRANCHINGS; p->name; ++p, ++count) {
    solver_.SetStrOption("var_branching", p->name);
    EXPECT_EQ(p->name, solver_.GetStrOption("var_branching"));
    EXPECT_EQ(p->value, solver_.var_branching());
  }
  EXPECT_EQ(24u, count);
}

TEST_F(NLSolverTest, DecayOption) {
  EXPECT_EQ(1, solver_.decay());
  solver_.SetDblOption("decay", 0.000001);
  EXPECT_EQ(0.000001, solver_.GetDblOption("decay"));
  EXPECT_EQ(0.000001, solver_.decay());
  solver_.SetDblOption("decay", 0.5);
  EXPECT_EQ(0.5, solver_.GetDblOption("decay"));
  EXPECT_EQ(0.5, solver_.decay());
  solver_.SetDblOption("decay", 1);
  EXPECT_EQ(1.0, solver_.decay());
  EXPECT_THROW(solver_.SetDblOption("decay", 0), InvalidOptionValue);
  EXPECT_THROW(solver_.SetDblOption("decay", 1.1), InvalidOptionValue);
}

const OptionValue<Gecode::RestartMode> RESTARTS[] = {
    {"none",      Gecode::RM_NONE},
    {"constant",  Gecode::RM_CONSTANT},
    {"linear",    Gecode::RM_LINEAR},
    {"luby",      Gecode::RM_LUBY},
    {"geometric", Gecode::RM_GEOMETRIC},
    {0,           Gecode::RM_NONE}
};

TEST_F(NLSolverTest, RestartOption) {
  EXPECT_EQ(Gecode::RM_NONE, solver_.restart());
  unsigned count = 0;
  for (const OptionValue<Gecode::RestartMode>
      *p = RESTARTS; p->name; ++p, ++count) {
    solver_.SetStrOption("restart", p->name);
    EXPECT_EQ(p->name, solver_.GetStrOption("restart"));
    EXPECT_EQ(p->value, solver_.restart());
  }
  EXPECT_EQ(5u, count);
}

TEST_F(NLSolverTest, RestartBaseOption) {
  EXPECT_EQ(1.5, solver_.restart_base());
  solver_.SetDblOption("restart_base", 0.000001);
  EXPECT_EQ(0.000001, solver_.GetDblOption("restart_base"));
  EXPECT_EQ(0.000001, solver_.restart_base());
  solver_.SetDblOption("restart_base", 0.5);
  EXPECT_EQ(0.5, solver_.GetDblOption("restart_base"));
  EXPECT_EQ(0.5, solver_.restart_base());
  solver_.SetDblOption("restart_base", 1);
  EXPECT_EQ(1.0, solver_.restart_base());
  solver_.SetDblOption("restart_base", -100);
  solver_.SetDblOption("restart_base", 100);
}

TEST_F(NLSolverTest, RestartScaleOption) {
  EXPECT_EQ(250u, solver_.restart_scale());
  solver_.SetIntOption("restart_scale", 42);
  EXPECT_EQ(42u, solver_.restart_scale());
  EXPECT_EQ(42, solver_.GetIntOption("restart_scale"));
  EXPECT_THROW(solver_.SetIntOption("restart_scale", -1), InvalidOptionValue);
}

TEST_F(NLSolverTest, OutLevOption) {
  TestOutputHandler h;
  solver_.set_output_handler(&h);
  Problem p;
  p.AddVar(0, 1, mp::var::INTEGER);
  p.AddObj(mp::obj::MIN, p.MakeBinary(mp::expr::ADD, p.MakeVariable(0),
                                      p.MakeNumericConstant(42)));
  mp::BasicSolutionHandler sh;
  solver_.Solve(p, sh);
  EXPECT_EQ("", h.output);

  h.output.clear();
  solver_.SetIntOption("outlev", 1);
  solver_.Solve(p, sh);
  EXPECT_EQ(
      " Max Depth      Nodes      Fails      Best Obj\n"
      "                                            42\n", h.output);

  solver_.SetIntOption("outlev", 0);
  EXPECT_EQ(0, solver_.GetIntOption("outlev"));
  solver_.SetIntOption("outlev", 1);
  EXPECT_EQ(1, solver_.GetIntOption("outlev"));
  EXPECT_THROW(solver_.SetIntOption("outlev", -1), InvalidOptionValue);
  EXPECT_THROW(solver_.SetIntOption("outlev", 2), InvalidOptionValue);
}

TEST_F(NLSolverTest, OutFreqOption) {
  TestOutputHandler h;
  solver_.set_output_handler(&h);
  Problem p;
  MakeTSP(p);
  solver_.SetIntOption("outlev", 1);
  solver_.SetDblOption("timelimit", 0.125);

  solver_.SetDblOption("outfreq", 0.05);
  mp::BasicSolutionHandler sh;
  solver_.Solve(p, sh);
  std::string out = h.output;
  EXPECT_EQ(4, std::count(out.begin(), out.end(), '\n'));

  h.output.clear();
  solver_.SetDblOption("outfreq", 0.1);
  solver_.Solve(p, sh);
  out = h.output;
  EXPECT_EQ(3, std::count(out.begin(), out.end(), '\n'));

  solver_.SetDblOption("outfreq", 1.23);
  EXPECT_EQ(1.23, solver_.GetDblOption("outfreq"));
  EXPECT_THROW(solver_.SetDblOption("outfreq", -1), InvalidOptionValue);
  EXPECT_THROW(solver_.SetDblOption("outfreq", 0), InvalidOptionValue);
}
