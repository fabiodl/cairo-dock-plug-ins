/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-wobbly.h"

#define l0 .33

static GLfloat pTexPts[2][2][2] = {{{0.0, 0.0}, {1.0, 0.0}}, {{0.0, 1.0}, {1.0, 1.0}}};
// static GLfloat pColorPts[2][2][4] = {{{1., 1., 1., 1.}, {1., 1., 1., 1.}}, {{1., 1., 1., 0.}, {1., 1., 1., 0.}}};
GLfloat colorPoints[4][4][4] =
{
	{
		{1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0}},
	{
		{1.0, 1.0, 1.0, .067},
		{1.0, 1.0, 1.0, .067},
		{1.0, 1.0, 1.0, .067},
		{1.0, 1.0, 1.0, .067}},
	{
		{1.0, 1.0, 1.0, .033},
		{1.0, 1.0, 1.0, .033},
		{1.0, 1.0, 1.0, .033},
		{1.0, 1.0, 1.0, .033}},
	{
		{1.0, 1.0, 1.0, .0},
		{1.0, 1.0, 1.0, .0},
		{1.0, 1.0, 1.0, .0},
		{1.0, 1.0, 1.0, .0}}
};

static void init (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL)
{
	if (bUseOpenGL)
	{
		double x, y, cx, cy;
		int i,j;
		for (i=0; i<4; i++)
		{
			x = (i-1.5)/3;  // -.5 -> .5
			cx = 1 + fabs (x);  // 1.5 -> 1
			for (j=0; j<4; j++)
			{
				y = - (j-1.5)/3;  // -.5 -> .5
				cy = 1 + fabs (y);  // 1.5 -> 1
				switch (myConfig.iInitialStrecth)
				{
					case CD_HORIZONTAL_STRECTH :
						pData->gridNodes[i][j].x = x * cx * cy;
						pData->gridNodes[i][j].y = y * cy;
					break ;
					
					case CD_VERTICAL_STRECTH :
						pData->gridNodes[i][j].x = x * cx;
						pData->gridNodes[i][j].y = y * cy * cx;
					break ;
					
					case CD_CORNER_STRECTH :
						pData->gridNodes[i][j].x = x * cx * cy/sqrt(2);
						pData->gridNodes[i][j].y = y * cy * cx/sqrt(2);
					break ;
					case CD_ANIMATIONS_NB_STRECTH :
					break ;
				}
				
				pData->gridNodes[i][j].vx = 0.;
				pData->gridNodes[i][j].vy = 0.;
			}
		}
	}
	else
	{
		pData->iWobblyCount = 20-1;
		if (pData->fWobblyWidthFactor == 0)
			pData->fWobblyWidthFactor = 1.;
		if (pData->fWobblyHeightFactor == 0)
			pData->fWobblyHeightFactor = 1.;
	}
}


#define _pulled_by2(i,j,s)\
	pNode2 = &pData->gridNodes[i][j];\
	dx = pNode2->x + pNode2->rk[s][2] - (pNode->x + pNode->rk[s][2]);\
	dy = pNode2->y + pNode2->rk[s][3] - (pNode->y + pNode->rk[s][3]);\
	l = sqrt (dx*dx + dy*dy);\
	if (0 && l > 1.5*l0) {\
	dx /= l/(l0/2/sqrt(2));\
	dy /= l/(l0/2/sqrt(2));\
	l = 1.5*l0;}\
	pNode->fx += k * dx * (1. - l0 / l);\
	pNode->fy += k * dy * (1. - l0 / l);\
	if (!bContinue && fabs (l - l0) > .005) bContinue = TRUE;

#define _pulled_by(i,j)\
	pNode2 = &pData->gridNodes[i][j];\
	dx = pNode2->x - pNode->x;\
	dy = pNode2->y - pNode->y;\
	l = sqrt (dx*dx + dy*dy);\
	if (0 && l > 1.5*l0) {\
	dx /= l/(l0/2/sqrt(2));\
	dy /= l/(l0/2/sqrt(2));\
	l = 1.5*l0;}\
	pNode->fx += k * dx * (1. - l0 / l);\
	pNode->fy += k * dy * (1. - l0 / l);\
	if (!bContinue && fabs (l - l0) > .005) bContinue = TRUE;

/*y' = f(t, y),
=> y_{n+1} = y_n + {h / 6} (k_1 + 2k_2 + 2k_3 + k_4)
    k_1 = f ( t_n, y_n )
    k_2 = f ( t_n + {h / 2}, y_n + {h / 2} k_1 )
    k_3 = f ( t_n + {h / 2}, y_n + {h / 2} k_2 )
    k_4 = f ( t_n + h, y_n + h k_3)
Ici : X' = f(Xn), avec X = {vx, vy, x, y} et f = {sum(Fx), sum(Fy), d/dt, d/dt}*/

static inline gboolean _calculate_forces (CDAnimationGridNode *pNode, int step, CDAnimationData *pData)
{
	double k = myConfig.fSpringConstant;
	double f = myConfig.fFriction;
	gboolean bContinue = FALSE;
	CDAnimationGridNode *pNode2;
	double dx, dy, l;
	int i,j;
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			pNode->fx = 0.;
			pNode->fy = 0.;
			
			if (i > 0)
			{
				_pulled_by2 (i-1, j, step);
			}
			if (i < 3)
			{
				_pulled_by2 (i+1, j, step);
			}
			if (j > 0)
			{
				_pulled_by2 (i, j-1, step);
			}
			if (j < 3)
			{
				_pulled_by2 (i, j+1, step);
			}
			
			pNode->fx -= f * (pNode->vx + pNode->rk[step][0]);
			pNode->fy -= f * (pNode->vy + pNode->rk[step][1]);
		}
	}
	return bContinue;
}

#define _sum_rk(i) (2*pNode->rk[1][i] + 4*pNode->rk[2][i] + 2*pNode->rk[3][i] + pNode->rk[4][i]) / 6.

gboolean cd_animations_update_wobbly2 (CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bWillContinue)
{
	CDAnimationGridNode *pNode;
	gboolean bContinue = FALSE;
	
	int i,j;
	// k_1 = f ( t_n, y_n ) = f(Xn)
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			pNode = &pData->gridNodes[i][j];
			
			bContinue |= _calculate_forces (pNode, 0, pData);
			pNode->rk[1][0] = pNode->fx * dt/2;
			pNode->rk[1][1] = pNode->fy * dt/2;
			pNode->rk[1][2] = pNode->vx * dt/2;
			pNode->rk[1][3] = pNode->vy * dt/2;
		}
	}
	// k_2 = f ( t_n + {h / 2}, y_n + {h / 2} k_1 ) = f(Xn + dt/2*k_1)
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			pNode = &pData->gridNodes[i][j];
			_calculate_forces (pNode, 1, pData);
			pNode->rk[2][0] = pNode->fx * dt/2;
			pNode->rk[2][1] = pNode->fy * dt/2;
			pNode->rk[2][2] = pNode->vx * dt/2;
			pNode->rk[2][3] = pNode->vy * dt/2;
		}
	}
	// k_3 = f ( t_n + {h / 2}, y_n + {h / 2} k_2 ) = f(Xn + dt/2*k_2)
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			pNode = &pData->gridNodes[i][j];
			_calculate_forces (pNode, 2, pData);
			pNode->rk[3][0] = pNode->fx * dt;
			pNode->rk[3][1] = pNode->fy * dt;
			pNode->rk[3][2] = pNode->vx * dt;
			pNode->rk[3][3] = pNode->vy * dt;
		}
	}
	// k_4 = f ( t_n + h, y_n + h k_3) = f(Xn + dt*k_3)
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			pNode = &pData->gridNodes[i][j];
			_calculate_forces (pNode, 3, pData);
			pNode->rk[4][0] = pNode->fx * dt;
			pNode->rk[4][1] = pNode->fy * dt;
			pNode->rk[4][2] = pNode->vx * dt;
			pNode->rk[4][3] = pNode->vy * dt;
		}
	}
	// y_{n+1} = y_n + {h / 6} (k_1 + 2k_2 + 2k_3 + k_4)
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			pNode = &pData->gridNodes[i][j];
			pNode->vx = _sum_rk (0);
			pNode->vy = _sum_rk (1);
			pNode->x = _sum_rk (2);
			pNode->y = _sum_rk (3);
		}
	}
	
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			pNode = &pData->gridNodes[i][j];
			pData->pCtrlPts[j][i][0] = pNode->x;
			pData->pCtrlPts[j][i][1] = pNode->y;
		}
	}
	
	cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));
	return bContinue;
}


static gboolean update (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL, gboolean bRepeat)
{
	gboolean bContinue = FALSE;
	if (bUseOpenGL)
	{
		const int n = 10;
		double k = myConfig.fSpringConstant;
		double f = myConfig.fFriction;
		dt /= 1e3 * n;
		CDAnimationGridNode *pNode, *pNode2;
		
		double dx, dy, l;
		int i,j,m;
		for (m=0; m<n; m++)
		{
			for (i=0; i<4; i++)
			{
				for (j=0; j<4; j++)
				{
					pNode = &pData->gridNodes[i][j];
					pNode->fx = 0.;
					pNode->fy = 0.;
					
					if (i > 0)
					{
						_pulled_by (i-1, j);
					}
					if (i < 3)
					{
						_pulled_by (i+1, j);
					}
					if (j > 0)
					{
						_pulled_by (i, j-1);
					}
					if (j < 3)
					{
						_pulled_by (i, j+1);
					}
				}
			}
			
			double _vx, _vy;
			for (i=0; i<4; i++)
			{
				for (j=0; j<4; j++)
				{
					pNode = &pData->gridNodes[i][j];
					pNode->fx -= f * pNode->vx;
					pNode->fy -= f * pNode->vy;
					
					_vx = pNode->vx;
					_vy = pNode->vy;
					pNode->vx += pNode->fx * dt;  // Runge-Kutta d'ordre 1.
					pNode->vy += pNode->fy * dt;
					
					pNode->x += (pNode->vx + _vx)/2 * dt;
					pNode->y += (pNode->vy + _vy)/2 * dt;
				}
			}
		}
		
		for (i=0; i<4; i++)
		{
			for (j=0; j<4; j++)
			{
				pNode = &pData->gridNodes[i][j];
				pData->pCtrlPts[j][i][0] = pNode->x;
				pData->pCtrlPts[j][i][1] = pNode->y;
			}
		}
		
		cairo_dock_redraw_container (CAIRO_CONTAINER (pDock));
	}
	else
	{
		int iNbIterInOneRound = 20;
		int c = pData->iWobblyCount;
		int n = iNbIterInOneRound / 4;  // nbre d'iteration pour 1 etirement/retrecissement.
		int k = c%n;
		
		double fDamageWidthFactor = (c == iNbIterInOneRound-1 ? 1. : pData->fWobblyWidthFactor);
		double fDamageHeightFactor = (c == iNbIterInOneRound-1 ? 1. : pData->fWobblyHeightFactor);
		double fMinSize = .3, fMaxSize = MIN (1.75, pDock->container.iHeight / pIcon->fWidth);  // au plus 1.75, soit 3/8 de l'icone qui deborde de part et d'autre de son emplacement. c'est suffisamment faible pour ne pas trop empieter sur ses voisines.
		
		double fSizeFactor = ((c/n) & 1 ? 1. / (n - k) : 1. / (1 + k));
		//double fSizeFactor = ((c/n) & 1 ? 1.*(k+1)/n : 1.*(n-k)/n);
		fSizeFactor = (fMinSize - fMaxSize) * fSizeFactor + fMaxSize;
		
		if ((c/(2*n)) & 1)
		{
			pData->fWobblyWidthFactor = fSizeFactor;
			pData->fWobblyHeightFactor = fMinSize;
			//g_print ("%d) width <- %.2f ; height <- %.2f (%d)\n", c, pData->fWidthFactor, pData->fHeightFactor, k);
		}
		else if (c != 0 || bRepeat)
		{
			pData->fWobblyHeightFactor = fSizeFactor;
			pData->fWobblyWidthFactor = fMinSize;
			//g_print ("%d) height <- %.2f ; width <- %.2f (%d)\n", c, pData->fHeightFactor, pData->fWidthFactor, k);
		}
		else
		{
			pData->fWobblyHeightFactor = 1.;
			pData->fWobblyWidthFactor = 1.;
		}
		pData->iWobblyCount --;
		
		if (! pDock->bIsShrinkingDown && ! pDock->bIsGrowingUp)
		{
			fDamageWidthFactor = MAX (fDamageWidthFactor, pData->fWobblyWidthFactor);
			fDamageHeightFactor = MAX (fDamageHeightFactor, pData->fWobblyHeightFactor);
			pIcon->fWidthFactor *= fDamageWidthFactor;
			pIcon->fHeightFactor *= fDamageHeightFactor;
			
			cairo_dock_redraw_icon (pIcon);
			
			pIcon->fWidthFactor /= fDamageWidthFactor;
			pIcon->fHeightFactor /= fDamageHeightFactor;
		}
		bContinue = (pData->iWobblyCount >= 0);
	}
	
	if (! bContinue && bRepeat)
		init (pIcon, pDock, pData, dt, bUseOpenGL);
	return bContinue;
}



static void render (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	if (pCairoContext)
	{
		pIcon->fWidthFactor *= pData->fWobblyWidthFactor;
		pIcon->fHeightFactor *= pData->fWobblyHeightFactor;
		cairo_save (pCairoContext);
		
		if (pDock->container.bIsHorizontal)
			cairo_translate (pCairoContext,
				pIcon->fWidth * pIcon->fScale * (1 - pIcon->fWidthFactor) / 2,
				pIcon->fHeight * pIcon->fScale * (1 - pIcon->fHeightFactor) / 2);
		else
			cairo_translate (pCairoContext,
				pIcon->fHeight * pIcon->fScale * (1 - pIcon->fHeightFactor) / 2,
				pIcon->fWidth * pIcon->fScale * (1 - pIcon->fWidthFactor) / 2);
		
		cairo_dock_draw_icon_cairo (pIcon, pDock, pCairoContext);
		
		cairo_restore (pCairoContext);
		
		pIcon->fWidthFactor /= pData->fWobblyWidthFactor;
		pIcon->fHeightFactor /= pData->fWobblyHeightFactor;
	}
	else
	{
		glPushMatrix ();
		cairo_dock_set_icon_scale (pIcon, CAIRO_CONTAINER (pDock), 1.);
		
		_cairo_dock_enable_texture ();  // enable texture, blend, and fill polygon
		_cairo_dock_set_blend_pbuffer ();
		_cairo_dock_set_alpha (pIcon->fAlpha);
		
		glBindTexture(GL_TEXTURE_2D, pIcon->image.iTexture);
		
		glEnable(GL_MAP2_VERTEX_3);  // active l'evaluateur 2D des sommets 3D
		glEnable(GL_MAP2_TEXTURE_COORD_2);
		
		glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4,
			0, 1, 12, 4, &pData->pCtrlPts[0][0][0]);
		glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
			0, 1, 4, 2, &pTexPts[0][0][0]);
		
		glMapGrid2f(myConfig.iNbGridNodes, 0.0, 1.0, myConfig.iNbGridNodes, 0.0, 1.0);  // Pour definir une grille reguliere de 0.0 a 1.0 en n etapes en u et m etapes en v
		glEvalMesh2(GL_FILL, 0, myConfig.iNbGridNodes, 0, myConfig.iNbGridNodes);  // Pour appliquer cette grille aux evaluateurs actives.
		glPopMatrix ();
		
		if (pDock->container.bUseReflect)
		{
			glPushMatrix ();
			// double fReflectRatio = pDock->iIconSize * myIconsParam.fReflectHeightRatio * pDock->container.fRatio / pIcon->fHeight / pIcon->fScale;
			///double fOffsetY = pIcon->fHeight * pIcon->fScale/2 + (pDock->iIconSize * myIconsParam.fReflectHeightRatio/2 + pIcon->fDeltaYReflection) * pDock->container.fRatio;
			double fOffsetY = pIcon->fHeight * pIcon->fScale + pIcon->fDeltaYReflection;
			if (pDock->container.bIsHorizontal)
			{
				if (pDock->container.bDirectionUp)
				{
					//fOffsetY = pIcon->fHeight * pIcon->fScale + pIcon->fDeltaYReflection;
					glTranslatef (0., - fOffsetY, 0.);
					glScalef (pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, - pIcon->fHeight * pIcon->fScale, 1.);  // taille du reflet et on se retourne.
				}
				else
				{
					glTranslatef (0., fOffsetY, 0.);
					glScalef (pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, - pIcon->fHeight * pIcon->fScale, 1.);
				}
			}
			else
			{
				if (pDock->container.bDirectionUp)
				{
					//fOffsetY = pIcon->fHeight * pIcon->fScale + pIcon->fDeltaYReflection;
					glTranslatef (fOffsetY, 0., 0.);
					glScalef (- pIcon->fHeight * pIcon->fScale, pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, 1.);
				}
				else
				{
					//fOffsetY = pIcon->fHeight * pIcon->fScale + pIcon->fDeltaYReflection;
					glTranslatef (- fOffsetY, 0., 0.);
					glScalef (- pIcon->fHeight * pIcon->fScale, pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, 1.);
				}
			}
			
			///glActiveTextureARB(GL_TEXTURE0_ARB); // Go pour le multitexturing 1ere passe
			glBindTexture(GL_TEXTURE_2D, pIcon->image.iTexture);
			glColor4f(1.0f, 1.0f, 1.0f, myIconsParam.fAlbedo * pIcon->fAlpha / 2);  // transparence du reflet.
			glBlendFuncSeparate (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
				GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_MAP2_TEXTURE_COORD_2);
			glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
				0, 1, 4, 2, &pTexPts[0][0][0]);
			
			/*glActiveTextureARB(GL_TEXTURE1_ARB); // Go pour le texturing 2eme passe
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, g_pGradationTexture[pDock->container.bIsHorizontal]);
			glColor4f(1.0f, 1.0f, 1.0f, myIconsParam.fAlbedo * pIcon->fAlpha);  // transparence du reflet.  // myIconsParam.fAlbedo * pIcon->fAlpha
			glEnable(GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // Le mode de combinaison des textures
			//glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);  // multiplier les alpha.
			glEnable(GL_MAP2_TEXTURE_COORD_2);
			glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2,
				0, 1, 4, 2, &pTexPts[0][0][0]);*/
			
			/*glEnable(GL_MAP2_COLOR_4);
			//glMap2f(GL_MAP2_COLOR_4, 0, 1, 4, 2,
			//	0, 1, 8, 2, &pColorPts[0][0][0]);
			glMap2f(GL_MAP2_COLOR_4, 0.0, 1.0, 4, 4,
				0.0, 1.0, 16, 4, &colorPoints[0][0][0]);*/
			glEvalMesh2(GL_FILL, 0, myConfig.iNbGridNodes, 0, myConfig.iNbGridNodes);
			
			/**glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);*/
			glDisable(GL_MAP2_COLOR_4);
			
			glPopMatrix ();
		}
		glDisable(GL_MAP2_VERTEX_3);
		glDisable(GL_MAP2_TEXTURE_COORD_2);
		_cairo_dock_disable_texture ();
	}
}


void cd_animations_register_wobbly (void)
{
	CDAnimation *pAnimation = &myData.pAnimations[CD_ANIMATIONS_WOBBLY];
	pAnimation->cName = "wobbly";
	pAnimation->cDisplayedName = D_("Wobbly");
	pAnimation->id = CD_ANIMATIONS_WOBBLY;
	pAnimation->bDrawIcon = TRUE;
	pAnimation->bDrawReflect = FALSE;
	pAnimation->init = init;
	pAnimation->update = update;
	pAnimation->render = render;
	pAnimation->post_render = NULL;
	cd_animations_register_animation (pAnimation);
}
