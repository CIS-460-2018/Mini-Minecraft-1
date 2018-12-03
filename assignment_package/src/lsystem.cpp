#include "lsystem.h"
#include <iostream>

LSystem::LSystem()
    :axiom(QString("FX")), turtle(Turtle()), turtleHistory(), savedStates()
{
    expansionOperations.insert('X', QString("FX"));
    turtleHistory.push(turtle);
//    Rule = &Rule1;
//    void (*LSystem::Rule)(void) = &drawLine;
//    drawingOperations.insert('X', LSystem::drawLine());
}

LSystem::LSystem(int x_boundary_start, int x_boundary_end, int z_boundary_start, int z_boundary_end)
    :axiom(QString("FFX")), turtle(Turtle(glm::vec2(x_boundary_start, 0), glm::vec2(1.0, 0.0), 1)), turtleHistory(), savedStates()
{
    expansionOperations.insert('X', QString("C[AFFBFFX]BFFBFFX"));
    turtleHistory.push(turtle);
}

LSystem::LSystem(QString axiom, int x_boundary_start, int x_boundary_end, int z_boundary_start, int z_boundary_end)
    :axiom(axiom), turtle(Turtle(glm::vec2(x_boundary_start, 0), glm::vec2(1.0, 0.0), 1)), turtleHistory(), savedStates()
{
    expansionOperations.insert('X', QString("FX"));
    turtleHistory.push(turtle);
}


QString LSystem::expandGrammar(const QString& s) const {
    QString new_s;
    int count = 0;
    while (count < s.length()) {
        if(expansionOperations.contains(s.at(count))) {
            new_s.append(expansionOperations.value(s.at(count)));
        }
        else {
            new_s.append(s.at(count));
        }
        count = count + 1;
    }
    return new_s;
}

void LSystem::executeRule(const QChar s) {
//    std::cout << "executing rule" << std::endl;
    if(s == QChar('F')) {
        drawLine();
    }
    if(s == QChar('A')) {
        rotateTurtleCW();
    }
    if(s == QChar('B')) {
        rotateTurtleCCW();
    }
    if(s == 'C') {
        savedStates.push_back(turtleHistory.last());
    }
    if(s == ']') {
        Turtle saved = savedStates.last();
        savedStates.pop_back();
        turtleHistory.push_back(saved);
        this->turtle = saved;
//        std::cout << turtle.pos.x << " " << turtle.pos.y << std::endl;
    }
}

void LSystem::drawLine() {
//    std::cout << "Drawing line" << std::endl;
    turtle.pos.x += 30 * turtle.orient.x;
    turtle.pos.y += 30 * turtle.orient.y;
    turtle.depth += 1;
    turtleHistory.push_back(turtle);
//    std::cout << "Depth " << turtle.depth << std::endl;
}

//void LSystem::branchTurtle() {


//}

void LSystem::branchTurtleCCW() {
    QStack<Turtle> new_history = turtleHistory;
    Turtle old_turtle = new_history.first();

    new_history.push_front(Turtle());
}

void LSystem::rotateTurtleCW() {
    //Positive x
    if(turtle.orient.x == 1.0)
    {
        turtle.orient.x = 0.5;
        turtle.orient.y = 0.5;
    }
    //Negative x
    else if(turtle.orient.x == -1.0)
    {
        turtle.orient.x = -0.5;
        turtle.orient.y = -0.5;
    }
    //Positive z
    else if(turtle.orient.x == 0.0 && turtle.orient.y == 1.0)
    {
        turtle.orient.x = -0.5;
        turtle.orient.y = 0.5;
    }
    //Negative z
    else if(turtle.orient.x == 0.0 && turtle.orient.y == -1.0)
    {
        turtle.orient.x = 0.5;
        turtle.orient.y = -0.5;
    }
    //Positive x and z
    else if(turtle.orient.x == 0.5 && turtle.orient.y == 0.5)
    {
        turtle.orient.x = 0.0;
        turtle.orient.y = 1.0;
    }
    //Positive x negative z
    else if(turtle.orient.x == 0.5 && turtle.orient.y == -0.5)
    {
        turtle.orient.x = 1.0;
        turtle.orient.y = 0.0;
    }
    //Negative x and z
    else if(turtle.orient.x == -0.5 && turtle.orient.y == -0.5)
    {
        turtle.orient.x = 0.0;
        turtle.orient.y = -1.0;
    }
    //Negative x positive z
    else if(turtle.orient.x == -0.5 && turtle.orient.y == 0.5)
    {
        turtle.orient.x = -1.0;
        turtle.orient.y = 0.0;
    }
    turtleHistory.push_back(turtle);
}

void LSystem::rotateTurtleCCW() {
    //Positive x
    if(turtle.orient.x == 1.0)
    {
        turtle.orient.x = 0.5;
        turtle.orient.y = -0.5;
    }
    //Negative x
    else if(turtle.orient.x == -1.0)
    {
        turtle.orient.x = -0.5;
        turtle.orient.y = 0.5;
    }
    //Positive z
    else if(turtle.orient.x == 0.0 && turtle.orient.y == 1.0)
    {
        turtle.orient.x = 0.5;
        turtle.orient.y = 0.5;
    }
    //Negative z
    else if(turtle.orient.x == 0.0 && turtle.orient.y == -1.0)
    {
        turtle.orient.x = -0.5;
        turtle.orient.y = -0.5;
    }
    //Positive x and z
    else if(turtle.orient.x == 0.5 && turtle.orient.y == 0.5)
    {
        turtle.orient.x = 1.0;
        turtle.orient.y = 0.0;
    }
    //Positive x negative z
    else if(turtle.orient.x == 0.5 && turtle.orient.y == -0.5)
    {
        turtle.orient.x = 0.0;
        turtle.orient.y = -1.0;
    }
    //Negative x and z
    else if(turtle.orient.x == -0.5 && turtle.orient.y == -0.5)
    {
        turtle.orient.x = -1.0;
        turtle.orient.y = 0.0;
    }
    //Negative x positive z
    else if(turtle.orient.x == -0.5 && turtle.orient.y == 0.5)
    {
        turtle.orient.x = 0.0;
        turtle.orient.y = 1.0;
    }
    turtleHistory.push_back(turtle);
}


