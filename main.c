#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define WIDTH 600
#define HEIGHT 600
#define SIZE 10

enum direction
{
  UP,
  DOWN,
  LEFT,
  RIGHT
} direction;

struct Node
{
  SDL_Rect rect;
  struct Node *next;
};

struct Apple
{
  SDL_Rect rect;
  int score;
  struct Node *next;
};

struct Node* MoveSnake(struct Node*, SDL_Rect, bool);

int main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
  {
    printf("Error initializing SDL: %s\n", SDL_GetError());
    return 0;
  }
  // Create a window
  SDL_Window *wind = SDL_CreateWindow("Snake",
                                      SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED,
                                      WIDTH, HEIGHT, 0);
  if (!wind)
  {
    printf("Error creating window: %s\n", SDL_GetError());
    SDL_Quit();
    return 0;
  }
  // Create a renderer
  SDL_Renderer *rend = SDL_CreateRenderer(wind, -1, 0);
  if (!rend)
  {
    printf("Error creating renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(wind);
    SDL_Quit();
    return 0;
  }

  // Init game vars and snake body
  int size = 0;
  int score = 0; 
  int dir = RIGHT;
  int FPS = 3;

  // create snake with 3 segments
  SDL_Rect head = {WIDTH / 2, HEIGHT / 2, SIZE, SIZE};
  struct Node butt;
  struct Node body;
  // Node pointer to keep track of butt of snake
  struct Node *p_butt = &butt;

  butt.rect.h = SIZE;
  butt.rect.w = SIZE;
  butt.rect.y = head.y;
  butt.rect.x = head.x + SIZE;
  butt.next = &body;

  body.rect.h = SIZE;
  body.rect.w = SIZE;
  body.rect.y = head.y;
  body.rect.x = butt.rect.x + SIZE;
  body.next = NULL;

  // Create apple

  struct Apple apple;
  apple.rect.h = SIZE;
  apple.rect.w = SIZE;
  apple.rect.x = rand() % 60 * SIZE;
  apple.rect.y = rand() % 60 * SIZE;
  apple.score = 5;
  apple.next = NULL;

  bool eaten = false;

  // Main loop
  SDL_Event event;
  bool running = true;
  bool pressed = false;

  while (running)
  {
    pressed = false;
    // Process events
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_KEYDOWN:

        if (!pressed)
        {
          switch (event.key.keysym.scancode)
          {
          case SDL_SCANCODE_A:
          case SDL_SCANCODE_LEFT:
          case SDL_SCANCODE_H:
            if (dir != RIGHT)
              dir = LEFT;
            break;
          case SDL_SCANCODE_D:
          case SDL_SCANCODE_RIGHT:
          case SDL_SCANCODE_L:
            if (dir != LEFT)
              dir = RIGHT;
            break;
          case SDL_SCANCODE_W:
          case SDL_SCANCODE_UP:
          case SDL_SCANCODE_K:
            if (dir != DOWN)
              dir = UP;
            break;
          case SDL_SCANCODE_S:
          case SDL_SCANCODE_DOWN:
          case SDL_SCANCODE_J:
            if (dir != UP)
              dir = DOWN;
            break;
          default:
            break;
          }
          pressed = true;
        }
        break;
      default:
        break;
      }
    }

    // check if collision with apple
    if (apple.rect.y == head.y && apple.rect.x == head.x)
    {
      eaten = true;
      score += apple.score;
      // TODO not inside snake
      apple.rect.x = rand() % 60 * SIZE;
      apple.rect.y = rand() % 60 * SIZE;
      apple.score = rand() % 20;
      if(apple.score > 10){
        FPS += 5;
      }else{
        FPS += 1;
      }
    }

    // check collision with self
    struct Node *p = p_butt;
    while(p != NULL){

      if(p->rect.x == head.x && p->rect.y == head.y){
        // TODO save score if highest
        // restart game 
        exit(1);
      }
      p = p->next;
    }


    // check collision with edge 
    if(head.x >= WIDTH || head.x < 0 || head.y >= HEIGHT || head.y < 0){
      exit(1);
    }


    p_butt = MoveSnake(p_butt, head, eaten);

    eaten = false;

    // Move snake in direction of dir
    switch (dir)
    {
    case UP:
      head.y -= SIZE;
      break;
    case DOWN:
      head.y += SIZE;
      break;
    case LEFT:
      head.x -= SIZE;
      break;
    case RIGHT:
      head.x += SIZE;
      break;

    default:
      break;
    }


    // print score in window title
    char score_str[256];
    sprintf(score_str, "Snake | Score: %d | Highscore: TODO | FPS: %d", score, FPS);
    SDL_SetWindowTitle(wind, score_str);


    // Clear screen
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_RenderClear(rend);


    // Draw the apple
    SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
    SDL_RenderFillRect(rend, &apple.rect);

    // NEEDS to be before body drawing
    // Draw the head
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    SDL_RenderFillRect(rend, &head);

    // Draw body segments of snake
    struct Node *segment = p_butt;
    while(segment != NULL){
      SDL_RenderFillRect(rend, &segment->rect);
      segment = segment->next;
    }

    // Draw to window and loop
    SDL_RenderPresent(rend);
    SDL_Delay(1000 / FPS);
  }
  // Release resources
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(wind);
  SDL_Quit();
  return 0;
}

// Moves snake by moving butt node to front of node list
// Then returns pointer to new butt of snake
struct Node *MoveSnake(struct Node *butt, SDL_Rect head, bool eaten)
{

  struct Node *p = butt;

  // make nodehead point to back as to back Node is going to become new nodehead
  do
  {
    if (p->next == NULL)
    {
      if (eaten)
      {
        // create new node with same pos as head
        struct Node *new_segment = (struct Node *)malloc(sizeof(struct Node));
        if(new_segment == NULL){
          printf("Failed to allocate new node\n");
          exit(-1);
        }
        new_segment->next = NULL;
        new_segment->rect = head;
        // make last body part point to new segment
        p->next = new_segment;
        return butt;
      }

      p->next = butt;
      break;
    }

    p = p->next;
  } while (p != NULL);

  p = butt->next;

  butt->rect = head;
  butt->next = NULL;

  // Return pointer to next segment after butt
  // to set p_butt
  return p;
}