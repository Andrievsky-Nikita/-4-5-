#include "functions.hpp"

int main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    FiniteAutomaton automaton = FiniteAutomaton::readFiniteAutomaton();
    automaton.printTransitionFunc();
    //FiniteAutomaton automatonWoutEpsilon = automaton.eliminateEpsilonTransitions();

}