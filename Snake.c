#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#define WIDTH 420
#define HEIGHT 300
#define SIZE 20

#define NUM_WAVE 2
const char* _waveFileNames[] = 
{
  "eat.wav",
  "die.wav"
};

Mix_Chunk* _sample[2];

enum direction
{
  UP,
  DOWN,
  LEFT,
  RIGHT
} direction;

typedef struct Node
{
  SDL_Rect rect;
  struct Node *next;
} Node;

typedef struct Apple
{
  SDL_Rect rect;
  struct Apple *next;
} Apple;

Node* MoveSnake(Node*, SDL_Rect, bool);

int Init(void)
{
  memset(_sample, 0, sizeof(Mix_Chunk *) * 2);

  // Set up the audio stream
  int result = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512);
  if (result < 0)
  {
    fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
    exit(1);
  }

  result = Mix_AllocateChannels(4);
  if (result < 0)
  {
    fprintf(stderr, "Unable to allocate mixing channels %s\n", SDL_GetError());
    exit(1);
  }

  // Load waveforms
  for (int i = 0; i < NUM_WAVE; i++)
  {
    _sample[i] = Mix_LoadWAV(_waveFileNames[i]);
    if (_sample[i] == NULL)
    {
      fprintf(stderr, "Unable to load wave file: %s Error: %s\n", _waveFileNames[i], Mix_GetError());
    }
  }
  return true;
}

int main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
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
  SDL_Renderer *rend = SDL_CreateRenderer(wind, -1, SDL_RENDERER_ACCELERATED);
  if (!rend)
  {
    printf("Error creating renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(wind);
    SDL_Quit();
    return 0;
  }
  
  if ( Init() == false){
    return 1;
  }

  // Init game vars
  int score = 0;
  int dir = RIGHT;
  int FPS = 8;
  int highscore = 0;
  int divX = WIDTH / SIZE;
  int divY = HEIGHT / SIZE;

  // Get highscore
  FILE *fptr = fopen("highscore.dat", "r");
  if (fptr != NULL)
  {
    fscanf(fptr, "%d", &highscore);
  }
  fclose(fptr);

  // create snake with 3 segments
  SDL_Rect head = {(WIDTH / 2) + ((WIDTH / 2) % SIZE), (HEIGHT / 2) + ((HEIGHT / 2) % SIZE), SIZE, SIZE};
  Node butt;
  Node body;
  // Node pointer to keep track of butt of snake
  Node *p_butt = &butt;

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
  apple.rect.x = rand() % divX * SIZE;
  apple.rect.y = rand() % divY * SIZE;
  apple.next = NULL;

  bool eaten = false;
  bool dead = false;

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
      Mix_PlayChannel(-1, _sample[0], 0);
      eaten = true;
      score += 5;
      // TODO not inside snake
      apple.rect.x = rand() % divX * SIZE;
      apple.rect.y = rand() % divY * SIZE;
    }

    // check collision with self
    Node *p = p_butt;
    while (p != NULL)
    {

      if (p->rect.x == head.x && p->rect.y == head.y)
      {
        // TODO save score if highest
        // restart game
        dead = true;
      }
      p = p->next;
    }

    // Check collision with edge
    if (head.x >= WIDTH || head.x < 0 || head.y < 0 || head.y >= HEIGHT)
    {
      dead = true;
    }

    if (dead){
      running = false;
      Mix_PlayChannel(-1, _sample[1], 0);
      while(Mix_Playing(-1) !=0){
        SDL_Delay(200);
      }
      break;
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

    // Print game info in window title
    char score_str[256];
    sprintf(score_str, "Snake | Score: %d | Highscore: %d", score, highscore);
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
    Node *segment = p_butt;
    while (segment != NULL)
    {
      SDL_RenderFillRect(rend, &segment->rect);
      segment = segment->next;
    }

    // Draw to window and loop
    SDL_RenderPresent(rend);
    SDL_Delay(1000 / FPS);
  }

  // Save score if bigger than highscore
  if (score > highscore)
  {
    FILE *fptr = fopen("highscore.dat", "w");
    if (fptr != NULL)
    {
      fprintf(fptr, "%d", score);
    }
    fclose(fptr);
  }

  // Release resources
  for ( int i = 0; i < NUM_WAVE; i++){
    Mix_FreeChunk(_sample[i]);
  }
  Mix_CloseAudio();
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(wind);
  SDL_Quit();
  return 0;
}

// Moves snake by moving butt node to front of node list
// Then returns pointer to new butt of snake
Node *MoveSnake(Node *butt, SDL_Rect head, bool eaten)
{

  Node *p = butt;

  // make nodehead point to back as to back Node is going to become new nodehead
  do
  {
    if (p->next == NULL)
    {
      if (eaten)
      {
        // create new node with same pos as head
        Node *new_segment = (Node *)malloc(sizeof(Node));
        if (new_segment == NULL)
        {
          printf("Failed to allocate new node\n");
          exit(1);
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
