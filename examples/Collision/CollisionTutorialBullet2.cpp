
#include "CollisionTutorialBullet2.h"
#include "../CommonInterfaces/CommonGraphicsAppInterface.h"
#include "../CommonInterfaces/CommonRenderInterface.h"

#include "../CommonInterfaces/CommonExampleInterface.h"
#include "LinearMath/btTransform.h"

#include "../CommonInterfaces/CommonGUIHelperInterface.h"
#include "../RenderingExamples/TimeSeriesCanvas.h"
#include "stb_image/stb_image.h"
#include "Bullet3Common/b3Quaternion.h"
#include "Bullet3Common/b3Matrix3x3.h"
#include "../CommonInterfaces/CommonParameterInterface.h"

#include "LinearMath/btAlignedObjectArray.h"
#include "CollisionSdkC_Api.h"


///Not Invented Here link reminder http://www.joelonsoftware.com/articles/fog0000000007.html

///todo: use the 'userData' to prevent this use of global variables
static int gTotalPoints = 0;
lwContactPoint pointsOut[10];
int pointCapacity=2;


void myNearCallback(plCollisionSdkHandle sdkHandle, plCollisionWorldHandle worldHandle, void* userData, plCollisionObjectHandle objA, plCollisionObjectHandle objB)
{
	int remainingCapacity = pointCapacity-gTotalPoints;
	btAssert(remainingCapacity>=0);

	if (remainingCapacity>0)
	{
		lwContactPoint* pointPtr = &pointsOut[gTotalPoints];
		int numNewPoints = plCollide(sdkHandle, worldHandle, objA,objB,pointPtr,remainingCapacity);
		btAssert(numNewPoints <= remainingCapacity);
		gTotalPoints+=numNewPoints;
	}
}

class CollisionTutorialBullet2 : public CommonExampleInterface
{
    CommonGraphicsApp* m_app;
	GUIHelperInterface* m_guiHelper;
    int m_tutorialIndex;
	
	TimeSeriesCanvas*			m_timeSeriesCanvas0;
	
	plCollisionSdkHandle m_collisionSdkHandle;
	plCollisionWorldHandle m_collisionWorldHandle;
	
	int m_stage;
	int m_counter;
public:
    
    CollisionTutorialBullet2(GUIHelperInterface* guiHelper, int tutorialIndex)
    :m_app(guiHelper->getAppInterface()),
	m_guiHelper(guiHelper),
	m_tutorialIndex(tutorialIndex),
	m_collisionSdkHandle(0),
	m_collisionWorldHandle(0),
	m_stage(0),
	m_counter(0),
	m_timeSeriesCanvas0(0)
    {
		int numBodies = 1;
		
		m_app->setUpAxis(1);
		m_app->m_renderer->enableBlend(true);
		
		switch (m_tutorialIndex)
		{
			case TUT_SPHERE_SPHERE:
			{
				numBodies=10;
				//m_collisionSdkHandle = plCreateBullet2CollisionSdk();
				m_collisionSdkHandle = plCreateRealTimeBullet3CollisionSdk();
				if (m_collisionSdkHandle)
				{
					int maxNumObjsCapacity=32;
					int maxNumShapesCapacity=1024;
					int maxNumPairsCapacity=16384;

					m_collisionWorldHandle = plCreateCollisionWorld(m_collisionSdkHandle,maxNumObjsCapacity,maxNumShapesCapacity,maxNumPairsCapacity);
					//create objects, do query etc
					float radius = 1.f;
					plCollisionShapeHandle colShape = plCreateSphereShape(m_collisionSdkHandle, m_collisionWorldHandle,radius);
					void* userPointer = 0;
                    btAlignedObjectArray<plCollisionObjectHandle> colliders;
                    int sphereGfxShapeId = m_app->registerGraphicsUnitSphereShape(SPHERE_LOD_HIGH);//, textureIndex);

                    for (int i=0;i<3;i++)
                    {
                        btVector3 pos(0,btScalar(i*1.5),0);
                        btQuaternion orn(0,0,0,1);
                        
						btVector4 color(0,1,0,0.8);
						btVector3 scaling(radius,radius,radius);
						
						int gfxIndex =  m_app->m_renderer->registerGraphicsInstance(sphereGfxShapeId,pos, orn,color,scaling);
						
						plCollisionObjectHandle colObj = plCreateCollisionObject(m_collisionSdkHandle,m_collisionWorldHandle,userPointer, gfxIndex,colShape,pos,orn);
                        colliders.push_back(colObj);
                        plAddCollisionObject(m_collisionSdkHandle, m_collisionWorldHandle,colObj);
                    }

                    int numContacts = plCollide(m_collisionSdkHandle,m_collisionWorldHandle,colliders[0],colliders[1],pointsOut,pointCapacity);
                    printf("numContacts = %d\n", numContacts);
                    void* myUserPtr = 0;
                    gTotalPoints = 0;
                    plWorldCollide(m_collisionSdkHandle,m_collisionWorldHandle,myNearCallback, myUserPtr);
                    printf("total points=%d\n",gTotalPoints);
                    
                    //plRemoveCollisionObject(m_collisionSdkHandle,m_collisionWorldHandle,colObj);
					//plDeleteCollisionObject(m_collisionSdkHandle,colObj);
					//plDeleteShape(m_collisionSdkHandle,colShape);
				}
				
				/*
				m_timeSeriesCanvas0 = new TimeSeriesCanvas(m_app->m_2dCanvasInterface,512,256,"Constant Velocity");
				
				m_timeSeriesCanvas0 ->setupTimeSeries(2,60, 0);
				m_timeSeriesCanvas0->addDataSource("X position (m)", 255,0,0);
				m_timeSeriesCanvas0->addDataSource("X velocity (m/s)", 0,0,255);
				m_timeSeriesCanvas0->addDataSource("dX/dt (m/s)", 0,0,0);
				 */
				break;
			}
			case TUT_SPHERE_PLANE:
			{
				break;
			}
			default:
			{
				
				m_timeSeriesCanvas0 = new TimeSeriesCanvas(m_app->m_2dCanvasInterface,512,256,"Unknown");
				m_timeSeriesCanvas0 ->setupTimeSeries(1,60, 0);
				
			}
		};

		
		
		if (m_tutorialIndex==TUT_SPHERE_SPHERE)
		{

		 int boxId = m_app->registerCubeShape(100,1,100);
            b3Vector3 pos = b3MakeVector3(0,-3.5,0);
            b3Quaternion orn(0,0,0,1);
            b3Vector4 color = b3MakeVector4(1,1,1,1);
            b3Vector3 scaling = b3MakeVector3(1,1,1);
            m_app->m_renderer->registerGraphicsInstance(boxId,pos,orn,color,scaling);
		}

		
		{
			int textureIndex = -1;
			
			if (1)
			{
				int width,height,n;
				
				const char* filename = "data/cube.png";
				const unsigned char* image=0;
				
				const char* prefix[]={"./","../","../../","../../../","../../../../"};
				int numprefix = sizeof(prefix)/sizeof(const char*);
				
				for (int i=0;!image && i<numprefix;i++)
				{
					char relativeFileName[1024];
					sprintf(relativeFileName,"%s%s",prefix[i],filename);
					image = stbi_load(relativeFileName, &width, &height, &n, 0);
				}
				
				b3Assert(image);
				if (image)
				{
					textureIndex = m_app->m_renderer->registerTexture(image,width,height);
				}
			}
			
		}
		
		m_app->m_renderer->writeTransforms();
    }
    virtual ~CollisionTutorialBullet2()
    {
		delete m_timeSeriesCanvas0;

		plDeleteCollisionWorld(m_collisionSdkHandle,m_collisionWorldHandle);

		plDeleteCollisionSdk(m_collisionSdkHandle);

		m_timeSeriesCanvas0 = 0;

		m_app->m_renderer->enableBlend(false);
    }
    
    
    virtual void    initPhysics()
    {
    }
    virtual void    exitPhysics()
    {
        
    }
	
	
    virtual void	stepSimulation(float deltaTime)
    {
		switch (m_tutorialIndex)
		{
			case TUT_SPHERE_SPHERE:
			{
				if (m_timeSeriesCanvas0)
				{
					float xPos = 0.f;
					float xVel = 1.f;
					m_timeSeriesCanvas0->insertDataAtCurrentTime(xPos,0,true);
					m_timeSeriesCanvas0->insertDataAtCurrentTime(xVel,1,true);
				}
				break;
			}
			

			default:
			{
			}
			
		};
		
		
		if (m_timeSeriesCanvas0)
			m_timeSeriesCanvas0->nextTick();
		
		
			
		//	m_app->m_renderer->writeSingleInstanceTransformToCPU(m_bodies[i]->m_worldPose.m_position, m_bodies[i]->m_worldPose.m_orientation, m_bodies[i]->m_graphicsIndex);
	
	
		 m_app->m_renderer->writeTransforms();
    }
    virtual void	renderScene()
    {
		if (m_app && m_app->m_renderer)
		{
			m_app->m_renderer->renderScene();

			m_app->m_renderer->clearZBuffer();

			m_app->drawText3D("X",1,0,0,1);
			m_app->drawText3D("Y",0,1,0,1);
			m_app->drawText3D("Z",0,0,1,1);


			for (int i=0;i<gTotalPoints;i++)
			{
				const lwContactPoint& contact = pointsOut[i];
				btVector3 color(1,1,0);
				btScalar lineWidth=3;
				if (contact.m_distance<0)
				{
					color.setValue(1,0,0);
				}
				m_app->m_renderer->drawLine(contact.m_ptOnAWorld,contact.m_ptOnBWorld,color,lineWidth);
			}
		}
		
    }

	

    virtual void	physicsDebugDraw(int debugDrawFlags)
    {
      
		
    }
    virtual bool	mouseMoveCallback(float x,float y)
    {
		return false;   
    }
    virtual bool	mouseButtonCallback(int button, int state, float x, float y)
    {
        return false;   
    }
    virtual bool	keyboardCallback(int key, int state)
    {
        return false;   
    }
  

	virtual void resetCamera()
	{
		float dist = 10.5;
		float pitch = 136;
		float yaw = 32;
		float targetPos[3]={0,0,0};
		if (m_app->m_renderer  && m_app->m_renderer->getActiveCamera())
		{
			m_app->m_renderer->getActiveCamera()->setCameraDistance(dist);
			m_app->m_renderer->getActiveCamera()->setCameraPitch(pitch);
			m_app->m_renderer->getActiveCamera()->setCameraYaw(yaw);
			m_app->m_renderer->getActiveCamera()->setCameraTargetPosition(targetPos[0],targetPos[1],targetPos[2]);
		}
	}
};

class CommonExampleInterface*    CollisionTutorialBullet2CreateFunc(struct CommonExampleOptions& options)
{
	return new CollisionTutorialBullet2(options.m_guiHelper, options.m_option);
}

