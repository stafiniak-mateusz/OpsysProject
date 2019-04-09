#include <iostream>
#include <thread>
#include <memory>
#include <vector>
#include <ncurses.h>
#include <cstdlib>
#include <stdlib.h>
#include <chrono>
#include <stdio.h>
#include <limits>
bool running = true;
int rows = 0;
int columns = 0;
int number_of_rabbits = 5;

void exit_on_key_down()
{
    while (running)
    {
        cbreak();
        noecho();
        char key = getch();

        if (key == 'q')
            running = false;
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
class Rabbit
{

    int x;
    int y;

  public:
    bool endThread = true;
    int get_x() { return this->x; }
    int get_y() { return this->y; }

    Rabbit(int x_pos, int y_pos)
    {
        this->x = x_pos;
        this->y = y_pos;
    }
    Rabbit()
    {
        this->x = 0;
        this->y = 0;
    };
    void move()
    {

        int direction = 0;
        bool flag = true;
        // bool endThread=true;
        while (running && endThread)
        { //TODO: Solve border case - rabbits' out of frame
            if (flag)
            {
                direction = rand() % 4;
            } //Current way does not work

            if (direction == 0)
            {
                this->x = this->x - 1;
                flag = true;
                if (this->x == 0)
                {
                    direction = 1;
                    flag = false;
                }
            }
            else if (direction == 1)
            {
                this->x = this->x + 1;
                flag = true;
                if (this->x == columns)
                {
                    direction = 0;
                    flag = false;
                }
            }
            else if (direction == 2)
            {
                this->y = this->y + 1;
                flag = true;
                if (this->y == rows)
                {
                    direction = 3;
                    flag = false;
                }
            }
            else if (direction == 3)
            {
                this->y = this->y - 1;
                flag = true;
                if (this->y == 0)
                {
                    direction = 2;
                    flag = false;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    std::thread moving_thread()
    {
        return std::thread(&Rabbit::move, this);
    }
};

std::vector<Rabbit *> rabbits;
std::vector<std::thread> rabbits_thread_array;

class Wolf
{
    int x;
    int y;

  public:
    int get_x() { return this->x; }
    int get_y() { return this->y; }
    Wolf(int x_pos, int y_pos)
    {
        this->x = x_pos;
        this->y = y_pos;
    }
    void seek()
    {
        float min;
        while (running)
        {
            min = std::numeric_limits<float>::max();
            Rabbit *closest_rabbit;
            for (auto r : rabbits)
            {
                if (rabbit_distance(r) < min)
                {
                    min = rabbit_distance(r);
                    closest_rabbit = r;
                }
            }
            automove(closest_rabbit);
            if (this->get_x() == closest_rabbit->get_x() && this->get_y() == closest_rabbit->get_y())
            {
                for (auto z : rabbits)
                {

                    if (z == closest_rabbit)
                    {
                        z->endThread = false;
                        delete z;
                        rabbits.shrink_to_fit();

                        number_of_rabbits--;
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    void automove(Rabbit *r)
    {
        if (r->get_x() > get_x())
        {
            this->x = get_x() + 1;
            return;
        }
        if (r->get_x() < get_x())
        {
            this->x = get_x() - 1;
            return;
        }
        if (r->get_y() > get_y())
        {
            this->y = get_y() + 1;
            return;
        }
        this->y = get_y() - 1;
    }
    float rabbit_distance(Rabbit *rabbit)
    {
        return abs((rabbit->get_x() - this->x) * (rabbit->get_x() - this->x) + (rabbit->get_y() - this->y) * (rabbit->get_y() - this->y));
    }
    std::thread seeking_thread()
    {
        return std::thread(&Wolf::seek, this);
    }
};

Wolf *global_wolf = NULL;
std::thread wolf_thread;

void insert_rabbit()
{

    getmaxyx(stdscr, rows, columns);
    global_wolf = new Wolf(rand() % rows, rand() % columns);
    wolf_thread = global_wolf->seeking_thread();

    while (number_of_rabbits--)
    {
        rabbits.push_back(new Rabbit(rand() % (rows - 1) + 1, rand() % (columns - 1) + 1));
        rabbits_thread_array.push_back(rabbits.back()->moving_thread());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void close_threads()
{
    for (int i = 0; i < rabbits_thread_array.size(); i++)
    {
        rabbits_thread_array[i].join();
    }
    wolf_thread.join();
}

void display()
{
    while (running)
    {
        clear();
        for (int i = 0; i < rabbits.size(); i++)
        {
            mvprintw(rabbits[i]->get_x(), rabbits[i]->get_y(), "R");
        }

        if (global_wolf == NULL)
        {
            std::cout << "";
        }
        else
        {

            start_color();
            init_pair(1, COLOR_CYAN, COLOR_BLACK);
            attron(COLOR_PAIR(1));
            mvprintw(global_wolf->get_x(), global_wolf->get_y(), "W");
            attroff(COLOR_PAIR(1));
        }
        if (number_of_rabbits == 1)
        {
            running = false;
            std::cout << "End of the simulation";
        }
        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
int main()
{
    srand(time(NULL));
    initscr();
    curs_set(0);

    std::thread create(insert_rabbit);
    std::thread map(display);
    std::thread exit_sim(exit_on_key_down);
    getch();

    map.join();
    create.join();

    close_threads();
    exit_sim.join();

    endwin();

    return 0;
}