#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#define WIDTH 640
#define HEIGHT 480
#define SIZE 15
#define FPS 1

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
  struct Node *next;
};

struct Node* MoveSnake(struct Node*, SDL_Rect);

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
  Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  SDL_Renderer *rend = SDL_CreateRenderer(wind, -1, render_flags);
  if (!rend)
  {
    printf("Error creating renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(wind);
    SDL_Quit();
    return 0;
  }

  // Main loop
  bool running = true;
  int size = 0;
  int score = 0; 
  int dir = RIGHT;
  SDL_Rect head = {WIDTH / 2, HEIGHT / 2, SIZE, SIZE};
  SDL_Event event;

  struct Node butt;
  struct Node body;
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

  while (running)
  {
    // Process events
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_KEYDOWN:
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
        break;
      default:
        break;
      }
    }

    // Move Snake
    p_butt = MoveSnake(p_butt, head);

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


    // Clear screen
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_RenderClear(rend);



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
struct Node* MoveSnake(struct Node* butt, SDL_Rect head){

  // Allocate for new node
  //struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));

  // make nodehead point to back as to back Node is going to become new nodehead
  struct Node *p = butt;
  do
  {
    if (p->next == NULL)
    {
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