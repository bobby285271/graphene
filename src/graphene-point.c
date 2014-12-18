/* graphene-point.c: Point
 *
 * Copyright © 2014  Emmanuele Bassi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * SECTION:graphene-point
 * @Title: Point
 * @short_description: A point with 2 coordinates
 *
 * #graphene_point_t is a data structure capable of describing a point with
 * two coordinates:
 *
 *  * @graphene_point_t.x
 *  * @graphene_point_t.y
 */

#include "graphene-private.h"
#include "graphene-point.h"
#include "graphene-simd4f.h"

#include <math.h>

/**
 * graphene_point_alloc: (constructor)
 *
 * Allocates a new #graphene_point_t structure.
 *
 * The coordinates of the returned point are (0, 0).
 *
 * It's possible to chain this function with graphene_point_init()
 * or graphene_point_init_from_point(), e.g.:
 *
 * |[<!-- language="C" -->
 *   graphene_point_t *
 *   point_new (float x, float y)
 *   {
 *     return graphene_point_init (graphene_point_alloc (), x, y);
 *   }
 *
 *   graphene_point_t *
 *   point_copy (const graphene_point_t *p)
 *   {
 *     return graphene_point_init_from_point (graphene_point_alloc (), p);
 *   }
 * ]|
 *
 * Returns: (transfer full): the newly allocated #graphene_point_t.
 *   Use graphene_point_free() to free the resources allocated by
 *   this function.
 *
 * Since: 1.0
 */
graphene_point_t *
graphene_point_alloc (void)
{
  return calloc (1, sizeof (graphene_point_t));
}

/**
 * graphene_point_free:
 * @p: a #graphene_point_t
 *
 * Frees the resources allocated by graphene_point_alloc().
 *
 * Since: 1.0
 */
void
graphene_point_free (graphene_point_t *p)
{
  free (p);
}

/**
 * graphene_point_init:
 * @p: a #graphene_point_t
 * @x: the X coordinate
 * @y: the Y coordinate
 *
 * Initializes @p to the given @x and @y coordinates.
 *
 * It's safe to call this function multiple times.
 *
 * Returns: (transfer none): the initialized point
 *
 * Since: 1.0
 */
graphene_point_t *
graphene_point_init (graphene_point_t *p,
                     float             x,
                     float             y)
{
  p->x = x;
  p->y = y;

  return p;
}

/**
 * graphene_point_init_from_point:
 * @p: a #graphene_point_t
 * @src: the #graphene_point_t to use
 *
 * Initializes @p with the same coordinates of @src.
 *
 * Returns: (transfer none): the initialized point
 *
 * Since: 1.0
 */
graphene_point_t *
graphene_point_init_from_point (graphene_point_t       *p,
                                const graphene_point_t *src)
{
  *p = *src;

  return p;
}

/**
 * graphene_point_equal:
 * @a: a #graphene_point_t
 * @b: a #graphene_point_t
 *
 * Checks if the two points @a and @b point to the same
 * coordinates.
 *
 * This function accounts for floating point fluctuations; if
 * you want to control the fuzziness of the match, you can use
 * graphene_point_near() instead.
 *
 * Returns: `true` if the points have the same coordinates
 *
 * Since: 1.0
 */
bool
graphene_point_equal (const graphene_point_t *a,
                      const graphene_point_t *b)
{
  if (a == b)
    return true;

  if (a == NULL || b == NULL)
    return false;

  return graphene_point_near (a, b, GRAPHENE_FLOAT_EPSILON);
}

/**
 * graphene_point_distance:
 * @a: a #graphene_point_t
 * @b: a #graphene_point_t
 * @d_x: (out) (optional): distance component on the X axis
 * @d_y: (out) (optional): distance component on the Y axis
 *
 * Computes the distance between @a and @b.
 *
 * Returns: the distance between the two points
 *
 * Since: 1.0
 */
float
graphene_point_distance (const graphene_point_t *a,
                         const graphene_point_t *b,
                         float                  *d_x,
                         float                  *d_y)
{
  graphene_simd4f_t s_a, s_b, res;

  if (a == b)
    return 0.f;

  s_a = graphene_simd4f_init (a->x, a->y, 0.f, 0.f);
  s_b = graphene_simd4f_init (b->x, b->y, 0.f, 0.f);
  res = graphene_simd4f_sub (s_a, s_b);

  if (d_x != NULL)
    *d_x = fabsf (graphene_simd4f_get_x (res));

  if (d_y != NULL)
    *d_y = fabsf (graphene_simd4f_get_y (res));

  return graphene_simd4f_get_x (graphene_simd4f_length2 (res));
}

/**
 * graphene_point_near:
 * @a: a #graphene_point_t
 * @b: a #graphene_point_t
 * @epsilon: threshold between the two points
 *
 * Checks whether the two points @a and @b are within
 * the threshold of @epsilon.
 *
 * Returns: `true` if the distance is within @epsilon
 *
 * Since: 1.0
 */
bool
graphene_point_near (const graphene_point_t *a,
                     const graphene_point_t *b,
                     float                   epsilon)
{
  graphene_simd4f_t s_a, s_b, res;

  if (a == b)
    return true;

  s_a = graphene_simd4f_init (a->x, a->y, 0.f, 0.f);
  s_b = graphene_simd4f_init (b->x, b->y, 0.f, 0.f);
  res = graphene_simd4f_sub (s_a, s_b);

  return fabsf (graphene_simd4f_get_x (res)) < epsilon &&
         fabsf (graphene_simd4f_get_y (res)) < epsilon;
}

/**
 * graphene_point_interpolate:
 * @a: a #graphene_point_t
 * @b: a #graphene_point_t
 * @factor: the linear interpolation factor
 * @res: (out caller-allocates): return location for the interpolated
 *   point
 *
 * Linearly interpolates the coordinates of @a and @b using the
 * given @factor.
 *
 * Since: 1.0
 */
void
graphene_point_interpolate (const graphene_point_t *a,
                            const graphene_point_t *b,
                            double                  factor,
                            graphene_point_t       *res)
{
  res->x = graphene_lerp (a->x, b->x, factor);
  res->y = graphene_lerp (a->y, b->y, factor);
}

static const graphene_point_t _graphene_point_zero = GRAPHENE_POINT_INIT_ZERO;

/**
 * graphene_point_zero:
 *
 * Returns a point fixed at (0, 0).
 *
 * Returns: (transfer none): a fixed point
 *
 * Since: 1.0
 */
const graphene_point_t *
graphene_point_zero (void)
{
  return &_graphene_point_zero;
}
