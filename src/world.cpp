//
// C Implementation: world
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "scene.h"
#include "screen.h"
#include "defines.h"
#include "world.h"
#include "tile.h"

texture_type img_distro[4];

void bouncy_distro_init(bouncy_distro_type* pbouncy_distro, float x, float y)
{
  pbouncy_distro->base.x = x;
  pbouncy_distro->base.y = y;
  pbouncy_distro->base.ym = -2;
}

void bouncy_distro_action(bouncy_distro_type* pbouncy_distro)
{
  pbouncy_distro->base.y = pbouncy_distro->base.y + pbouncy_distro->base.ym * frame_ratio;

  pbouncy_distro->base.ym += 0.1 * frame_ratio;

  if (pbouncy_distro->base.ym >= 0)
    bouncy_distros.erase(static_cast<std::vector<bouncy_distro_type>::iterator>(pbouncy_distro));
}

void bouncy_distro_draw(bouncy_distro_type* pbouncy_distro)
{
  texture_draw(&img_distro[0],
               pbouncy_distro->base.x - scroll_x,
               pbouncy_distro->base.y);
}

void broken_brick_init(broken_brick_type* pbroken_brick, float x, float y, float xm, float ym)
{
  pbroken_brick->base.x = x;
  pbroken_brick->base.y = y;
  pbroken_brick->base.xm = xm;
  pbroken_brick->base.ym = ym;
  timer_init(&pbroken_brick->timer, true);
  timer_start(&pbroken_brick->timer,200);
}

void broken_brick_action(broken_brick_type* pbroken_brick)
{
  pbroken_brick->base.x = pbroken_brick->base.x + pbroken_brick->base.xm * frame_ratio;
  pbroken_brick->base.y = pbroken_brick->base.y + pbroken_brick->base.ym * frame_ratio;

  if (!timer_check(&pbroken_brick->timer))
    broken_bricks.erase(static_cast<std::vector<broken_brick_type>::iterator>(pbroken_brick));
}

void broken_brick_draw(broken_brick_type* pbroken_brick)
{
  SDL_Rect src, dest;
  src.x = rand() % 16;
  src.y = rand() % 16;
  src.w = 16;
  src.h = 16;

  dest.x = (int)(pbroken_brick->base.x - scroll_x);
  dest.y = (int)pbroken_brick->base.y;
  dest.w = 16;
  dest.h = 16;

  texture_draw_part(&img_brick[0],src.x,src.y,dest.x,dest.y,dest.w,dest.h);
}

void bouncy_brick_init(bouncy_brick_type* pbouncy_brick, float x, float y)
{
  pbouncy_brick->base.x   = x;
  pbouncy_brick->base.y   = y;
  pbouncy_brick->offset   = 0;
  pbouncy_brick->offset_m = -BOUNCY_BRICK_SPEED;
  pbouncy_brick->shape    = GameSession::current()->get_level()->gettileid(x, y);
}

void bouncy_brick_action(bouncy_brick_type* pbouncy_brick)
{

  pbouncy_brick->offset = (pbouncy_brick->offset +
                           pbouncy_brick->offset_m * frame_ratio);

  /* Go back down? */

  if (pbouncy_brick->offset < -BOUNCY_BRICK_MAX_OFFSET)
    pbouncy_brick->offset_m = BOUNCY_BRICK_SPEED;


  /* Stop bouncing? */

  if (pbouncy_brick->offset >= 0)
    bouncy_bricks.erase(static_cast<std::vector<bouncy_brick_type>::iterator>(pbouncy_brick));
}

void bouncy_brick_draw(bouncy_brick_type* pbouncy_brick)
{
  int s;
  SDL_Rect dest;
  
  if (pbouncy_brick->base.x >= scroll_x - 32 &&
      pbouncy_brick->base.x <= scroll_x + screen->w)
    {
      dest.x = (int)(pbouncy_brick->base.x - scroll_x);
      dest.y = (int)pbouncy_brick->base.y;
      dest.w = 32;
      dest.h = 32;

      Level* plevel = GameSession::current()->get_level();

      // FIXME: overdrawing hack to clean the tile from the screen to
      // paint it later at on offseted position
      if(plevel->bkgd_image[0] == '\0')
        {
          fillrect(pbouncy_brick->base.x - scroll_x, pbouncy_brick->base.y,
                   32,32, 
                   plevel->bkgd_red, plevel->bkgd_green, plevel->bkgd_blue, 0);
        }
      else
        {
          s = (int)scroll_x / 30;
          texture_draw_part(&img_bkgd,dest.x + s,dest.y,dest.x,dest.y,dest.w,dest.h);
        }

      drawshape(pbouncy_brick->base.x - scroll_x,
                pbouncy_brick->base.y + pbouncy_brick->offset,
                pbouncy_brick->shape);
    }
}

void floating_score_init(floating_score_type* pfloating_score, float x, float y, int s)
{
  pfloating_score->base.x = x;
  pfloating_score->base.y = y - 16;
  timer_init(&pfloating_score->timer,true);
  timer_start(&pfloating_score->timer,1000);
  pfloating_score->value = s;
}

void floating_score_action(floating_score_type* pfloating_score)
{
  pfloating_score->base.y = pfloating_score->base.y - 2 * frame_ratio;

  if(!timer_check(&pfloating_score->timer))
    floating_scores.erase(static_cast<std::vector<floating_score_type>::iterator>(pfloating_score));
}

void floating_score_draw(floating_score_type* pfloating_score)
{
  char str[10];
  sprintf(str, "%d", pfloating_score->value);
  text_draw(&gold_text, str, (int)pfloating_score->base.x + 16 - strlen(str) * 8, (int)pfloating_score->base.y, 1);
}

/* Break a brick: */
void trybreakbrick(float x, float y, bool small)
{
  Level* plevel = GameSession::current()->get_level();
  
  Tile* tile = gettile(x, y);
  if (tile->brick)
    {
      if (tile->data > 0)
        {
          /* Get a distro from it: */
          add_bouncy_distro(((int)(x + 1) / 32) * 32,
                            (int)(y / 32) * 32);

          if (!counting_distros)
            {
              counting_distros = true;
              distro_counter = 50;
            }

          if (distro_counter <= 0)
            plevel->change(x, y, TM_IA, tile->next_tile);

          play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
          score = score + SCORE_DISTRO;
          distros++;
        }
      else if (!small)
        {
          /* Get rid of it: */
          plevel->change(x, y, TM_IA, tile->next_tile);
          
          /* Replace it with broken bits: */
          add_broken_brick(((int)(x + 1) / 32) * 32,
                           (int)(y / 32) * 32);
          
          /* Get some score: */
          play_sound(sounds[SND_BRICK], SOUND_CENTER_SPEAKER);
          score = score + SCORE_BRICK;
        }
    }
}

/* Empty a box: */
void tryemptybox(float x, float y, int col_side)
{
  Level* plevel = GameSession::current()->get_level();

  Tile* tile = gettile(x,y);
  if (!tile->fullbox)
    return;

  // according to the collision side, set the upgrade direction
  if(col_side == LEFT)
    col_side = RIGHT;
  else
    col_side = LEFT;

  switch(tile->data)
    {
    case 1: //'A':      /* Box with a distro! */
      add_bouncy_distro(((int)(x + 1) / 32) * 32, (int)(y / 32) * 32 - 32);
      play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
      score = score + SCORE_DISTRO;
      distros++;
      break;

    case 2: // 'B':      /* Add an upgrade! */
      if (tux.size == SMALL)     /* Tux is small, add mints! */
        add_upgrade((int)((x + 1) / 32) * 32, (int)(y / 32) * 32 - 32, col_side, UPGRADE_MINTS);
      else     /* Tux is big, add coffee: */
        add_upgrade((int)((x + 1) / 32) * 32, (int)(y / 32) * 32 - 32, col_side, UPGRADE_COFFEE);
      play_sound(sounds[SND_UPGRADE], SOUND_CENTER_SPEAKER);
      break;

    case 3:// '!':     /* Add a golden herring */
      add_upgrade((int)((x + 1) / 32) * 32, (int)(y / 32) * 32 - 32, col_side, UPGRADE_HERRING);
      break;
    default:
      break;
    }

  /* Empty the box: */
  plevel->change(x, y, TM_IA, tile->next_tile);
}

/* Try to grab a distro: */
void trygrabdistro(float x, float y, int bounciness)
{
  Level* plevel = GameSession::current()->get_level();
  Tile* tile = gettile(x, y);
  if (tile && tile->distro)
    {
      plevel->change(x, y, TM_IA, tile->next_tile);
      play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);

      if (bounciness == BOUNCE)
        {
          add_bouncy_distro(((int)(x + 1) / 32) * 32,
                            (int)(y / 32) * 32);
        }

      score = score + SCORE_DISTRO;
      distros++;
    }
}

/* Try to bump a bad guy from below: */
void trybumpbadguy(float x, float y)
{
  /* Bad guys: */
  for (unsigned int i = 0; i < bad_guys.size(); i++)
    {
      if (bad_guys[i].base.x >= x - 32 && bad_guys[i].base.x <= x + 32 &&
          bad_guys[i].base.y >= y - 16 && bad_guys[i].base.y <= y + 16)
        {
          bad_guys[i].collision(&tux, CO_PLAYER, COLLISION_BUMP);
        }
    }


  /* Upgrades: */
  for (unsigned int i = 0; i < upgrades.size(); i++)
    {
      if (upgrades[i].base.height == 32 &&
          upgrades[i].base.x >= x - 32 && upgrades[i].base.x <= x + 32 &&
          upgrades[i].base.y >= y - 16 && upgrades[i].base.y <= y + 16)
        {
          upgrades[i].base.xm = -upgrades[i].base.xm;
          upgrades[i].base.ym = -8;
          play_sound(sounds[SND_BUMP_UPGRADE], SOUND_CENTER_SPEAKER);
        }
    }
}

/* EOF */

