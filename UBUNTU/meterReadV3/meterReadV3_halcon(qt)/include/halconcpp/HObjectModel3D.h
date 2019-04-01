/***********************************************************
 * File generated by the HALCON-Compiler hcomp version 12.0
 * Usage: Interface to C++
 *
 * Software by: MVTec Software GmbH, www.mvtec.com
 ***********************************************************/


#ifndef HCPP_HOBJECTMODEL3D
#define HCPP_HOBJECTMODEL3D

namespace HalconCpp
{

// Represents an instance of a 3D object model.
class LIntExport HObjectModel3D : public HToolBase
{

public:

  // Copy constructor
  HObjectModel3D(const HObjectModel3D& source) : HToolBase(source) {}

  // Create HObjectModel3D from handle, taking ownership
  explicit HObjectModel3D(Hlong handle);

  // Set new handle, taking ownership
  void SetHandle(Hlong handle);

  // Deep copy of all data represented by this object instance
  HObjectModel3D Clone() const;



/*****************************************************************************
 * Operator-based class constructors
 *****************************************************************************/

  // gen_empty_object_model_3d: Create an empty 3D object model.
  explicit HObjectModel3D();

  // gen_object_model_3d_from_points: Create a 3D object model that represents a point cloud from a set of 3D points.
  explicit HObjectModel3D(const HTuple& X, const HTuple& Y, const HTuple& Z);

  // gen_object_model_3d_from_points: Create a 3D object model that represents a point cloud from a set of 3D points.
  explicit HObjectModel3D(double X, double Y, double Z);

  // xyz_to_object_model_3d: Transform 3D points from images to a 3D object model.
  explicit HObjectModel3D(const HImage& X, const HImage& Y, const HImage& Z);

  // read_object_model_3d: Read a 3D object model from a file.
  explicit HObjectModel3D(const HString& FileName, const HTuple& Scale, const HTuple& GenParamName, const HTuple& GenParamValue, HTuple* Status);

  // read_object_model_3d: Read a 3D object model from a file.
  explicit HObjectModel3D(const HString& FileName, const HString& Scale, const HString& GenParamName, const HString& GenParamValue, HString* Status);

  // read_object_model_3d: Read a 3D object model from a file.
  explicit HObjectModel3D(const char* FileName, const char* Scale, const char* GenParamName, const char* GenParamValue, HString* Status);




  /***************************************************************************
   * Operators                                                               *
   ***************************************************************************/

  // Get the result of a calibrated measurement performed with the  sheet-of-light technique as a 3D object model.
  void GetSheetOfLightResultObjectModel3d(const HSheetOfLightModel& SheetOfLightModelID);

  // Fit 3D primitives into a set of 3D points.
  static HObjectModel3DArray FitPrimitivesObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HTuple& ParamName, const HTuple& ParamValue);

  // Fit 3D primitives into a set of 3D points.
  HObjectModel3D FitPrimitivesObjectModel3d(const HString& ParamName, const HString& ParamValue) const;

  // Fit 3D primitives into a set of 3D points.
  HObjectModel3D FitPrimitivesObjectModel3d(const char* ParamName, const char* ParamValue) const;

  // Segment a set of 3D points into sub-sets with similar characteristics.
  static HObjectModel3DArray SegmentObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HTuple& ParamName, const HTuple& ParamValue);

  // Segment a set of 3D points into sub-sets with similar characteristics.
  HObjectModel3D SegmentObjectModel3d(const HString& ParamName, const HString& ParamValue) const;

  // Segment a set of 3D points into sub-sets with similar characteristics.
  HObjectModel3D SegmentObjectModel3d(const char* ParamName, const char* ParamValue) const;

  // Calculate the 3D surface normals of a 3D object model.
  static HObjectModel3DArray SurfaceNormalsObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HString& Method, const HTuple& GenParamName, const HTuple& GenParamValue);

  // Calculate the 3D surface normals of a 3D object model.
  HObjectModel3D SurfaceNormalsObjectModel3d(const HString& Method, const HString& GenParamName, double GenParamValue) const;

  // Calculate the 3D surface normals of a 3D object model.
  HObjectModel3D SurfaceNormalsObjectModel3d(const char* Method, const char* GenParamName, double GenParamValue) const;

  // Smooth the 3D points of a 3D object model.
  static HObjectModel3DArray SmoothObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HString& Method, const HTuple& GenParamName, const HTuple& GenParamValue);

  // Smooth the 3D points of a 3D object model.
  HObjectModel3D SmoothObjectModel3d(const HString& Method, const HString& GenParamName, double GenParamValue) const;

  // Smooth the 3D points of a 3D object model.
  HObjectModel3D SmoothObjectModel3d(const char* Method, const char* GenParamName, double GenParamValue) const;

  // Create a surface triangulation for a 3D object model.
  static HObjectModel3DArray TriangulateObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HString& Method, const HTuple& GenParamName, const HTuple& GenParamValue, HTuple* Information);

  // Create a surface triangulation for a 3D object model.
  HObjectModel3D TriangulateObjectModel3d(const HString& Method, const HString& GenParamName, double GenParamValue, Hlong* Information) const;

  // Create a surface triangulation for a 3D object model.
  HObjectModel3D TriangulateObjectModel3d(const char* Method, const char* GenParamName, double GenParamValue, Hlong* Information) const;

  // Reconstruct surface from calibrated multi-view stereo images.
  void ReconstructSurfaceStereo(const HImage& Images, const HStereoModel& StereoModelID);

  // Refine the position and deformation of a deformable surface model in a 3D scene.
  HTuple RefineDeformableSurfaceModel(const HDeformableSurfaceModel& DeformableSurfaceModel, double RelSamplingDistance, const HObjectModel3D& InitialDeformationObjectModel3D, const HTuple& GenParamName, const HTuple& GenParamValue, HDeformableSurfaceMatchingResultArray* DeformableSurfaceMatchingResult) const;

  // Refine the position and deformation of a deformable surface model in a 3D scene.
  double RefineDeformableSurfaceModel(const HDeformableSurfaceModel& DeformableSurfaceModel, double RelSamplingDistance, const HObjectModel3D& InitialDeformationObjectModel3D, const HString& GenParamName, const HString& GenParamValue, HDeformableSurfaceMatchingResult* DeformableSurfaceMatchingResult) const;

  // Refine the position and deformation of a deformable surface model in a 3D scene.
  double RefineDeformableSurfaceModel(const HDeformableSurfaceModel& DeformableSurfaceModel, double RelSamplingDistance, const HObjectModel3D& InitialDeformationObjectModel3D, const char* GenParamName, const char* GenParamValue, HDeformableSurfaceMatchingResult* DeformableSurfaceMatchingResult) const;

  // Find the best match of a deformable surface model in a 3D scene.
  HTuple FindDeformableSurfaceModel(const HDeformableSurfaceModel& DeformableSurfaceModel, double RelSamplingDistance, const HTuple& MinScore, const HTuple& GenParamName, const HTuple& GenParamValue, HDeformableSurfaceMatchingResultArray* DeformableSurfaceMatchingResult) const;

  // Find the best match of a deformable surface model in a 3D scene.
  double FindDeformableSurfaceModel(const HDeformableSurfaceModel& DeformableSurfaceModel, double RelSamplingDistance, double MinScore, const HTuple& GenParamName, const HTuple& GenParamValue, HDeformableSurfaceMatchingResult* DeformableSurfaceMatchingResult) const;

  // Add a sample deformation to a deformable surface model
  static void AddDeformableSurfaceModelSample(const HDeformableSurfaceModel& DeformableSurfaceModel, const HObjectModel3DArray& ObjectModel3D);

  // Add a sample deformation to a deformable surface model
  void AddDeformableSurfaceModelSample(const HDeformableSurfaceModel& DeformableSurfaceModel) const;

  // Create the data structure needed to perform deformable surface-based matching.
  HDeformableSurfaceModel CreateDeformableSurfaceModel(double RelSamplingDistance, const HTuple& GenParamName, const HTuple& GenParamValue) const;

  // Create the data structure needed to perform deformable surface-based matching.
  HDeformableSurfaceModel CreateDeformableSurfaceModel(double RelSamplingDistance, const HString& GenParamName, const HString& GenParamValue) const;

  // Create the data structure needed to perform deformable surface-based matching.
  HDeformableSurfaceModel CreateDeformableSurfaceModel(double RelSamplingDistance, const char* GenParamName, const char* GenParamValue) const;

  // Refine the pose of a surface model in a 3D scene.
  HPoseArray RefineSurfaceModelPose(const HSurfaceModel& SurfaceModelID, const HPoseArray& InitialPose, const HTuple& MinScore, const HString& ReturnResultHandle, const HTuple& GenParamName, const HTuple& GenParamValue, HTuple* Score, HSurfaceMatchingResultArray* SurfaceMatchingResultID) const;

  // Refine the pose of a surface model in a 3D scene.
  HPose RefineSurfaceModelPose(const HSurfaceModel& SurfaceModelID, const HPose& InitialPose, double MinScore, const HString& ReturnResultHandle, const HTuple& GenParamName, const HTuple& GenParamValue, HTuple* Score, HSurfaceMatchingResult* SurfaceMatchingResultID) const;

  // Refine the pose of a surface model in a 3D scene.
  HPose RefineSurfaceModelPose(const HSurfaceModel& SurfaceModelID, const HPose& InitialPose, double MinScore, const char* ReturnResultHandle, const HTuple& GenParamName, const HTuple& GenParamValue, HTuple* Score, HSurfaceMatchingResult* SurfaceMatchingResultID) const;

  // Find the best matches of a surface model in a 3D scene.
  HPoseArray FindSurfaceModel(const HSurfaceModel& SurfaceModelID, double RelSamplingDistance, double KeyPointFraction, const HTuple& MinScore, const HString& ReturnResultHandle, const HTuple& GenParamName, const HTuple& GenParamValue, HTuple* Score, HSurfaceMatchingResultArray* SurfaceMatchingResultID) const;

  // Find the best matches of a surface model in a 3D scene.
  HPose FindSurfaceModel(const HSurfaceModel& SurfaceModelID, double RelSamplingDistance, double KeyPointFraction, double MinScore, const HString& ReturnResultHandle, const HTuple& GenParamName, const HTuple& GenParamValue, HTuple* Score, HSurfaceMatchingResult* SurfaceMatchingResultID) const;

  // Find the best matches of a surface model in a 3D scene.
  HPose FindSurfaceModel(const HSurfaceModel& SurfaceModelID, double RelSamplingDistance, double KeyPointFraction, double MinScore, const char* ReturnResultHandle, const HTuple& GenParamName, const HTuple& GenParamValue, HTuple* Score, HSurfaceMatchingResult* SurfaceMatchingResultID) const;

  // Create the data structure needed to perform surface-based matching.
  HSurfaceModel CreateSurfaceModel(double RelSamplingDistance, const HTuple& GenParamName, const HTuple& GenParamValue) const;

  // Create the data structure needed to perform surface-based matching.
  HSurfaceModel CreateSurfaceModel(double RelSamplingDistance, const HString& GenParamName, const HString& GenParamValue) const;

  // Create the data structure needed to perform surface-based matching.
  HSurfaceModel CreateSurfaceModel(double RelSamplingDistance, const char* GenParamName, const char* GenParamValue) const;

  // Simplify a triangulated 3D object model.
  static HObjectModel3DArray SimplifyObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HString& Method, const HTuple& Amount, const HTuple& GenParamName, const HTuple& GenParamValue);

  // Simplify a triangulated 3D object model.
  HObjectModel3D SimplifyObjectModel3d(const HString& Method, double Amount, const HString& GenParamName, const HString& GenParamValue) const;

  // Simplify a triangulated 3D object model.
  HObjectModel3D SimplifyObjectModel3d(const char* Method, double Amount, const char* GenParamName, const char* GenParamValue) const;

  // Compute the distances of the points of one 3D object model to another 3D object model.
  void DistanceObjectModel3d(const HObjectModel3D& ObjectModel3DTo, const HPose& Pose, const HTuple& MaxDistance, const HTuple& GenParamNames, const HTuple& GenParamValues) const;

  // Compute the distances of the points of one 3D object model to another 3D object model.
  void DistanceObjectModel3d(const HObjectModel3D& ObjectModel3DTo, const HPose& Pose, double MaxDistance, const HString& GenParamNames, const HString& GenParamValues) const;

  // Compute the distances of the points of one 3D object model to another 3D object model.
  void DistanceObjectModel3d(const HObjectModel3D& ObjectModel3DTo, const HPose& Pose, double MaxDistance, const char* GenParamNames, const char* GenParamValues) const;

  // Combine several 3D object models to a new 3D object model.
  static HObjectModel3D UnionObjectModel3d(const HObjectModel3DArray& ObjectModels3D, const HString& Method);

  // Combine several 3D object models to a new 3D object model.
  HObjectModel3D UnionObjectModel3d(const HString& Method) const;

  // Combine several 3D object models to a new 3D object model.
  HObjectModel3D UnionObjectModel3d(const char* Method) const;

  // Set attributes of a 3D object model.
  void SetObjectModel3dAttribMod(const HTuple& AttribName, const HString& AttachExtAttribTo, const HTuple& AttribValues) const;

  // Set attributes of a 3D object model.
  void SetObjectModel3dAttribMod(const HString& AttribName, const HString& AttachExtAttribTo, double AttribValues) const;

  // Set attributes of a 3D object model.
  void SetObjectModel3dAttribMod(const char* AttribName, const char* AttachExtAttribTo, double AttribValues) const;

  // Set attributes of a 3D object model.
  HObjectModel3D SetObjectModel3dAttrib(const HTuple& AttribName, const HString& AttachExtAttribTo, const HTuple& AttribValues) const;

  // Set attributes of a 3D object model.
  HObjectModel3D SetObjectModel3dAttrib(const HString& AttribName, const HString& AttachExtAttribTo, double AttribValues) const;

  // Set attributes of a 3D object model.
  HObjectModel3D SetObjectModel3dAttrib(const char* AttribName, const char* AttachExtAttribTo, double AttribValues) const;

  // Create an empty 3D object model.
  void GenEmptyObjectModel3d();

  // Sample a 3D object model.
  static HObjectModel3DArray SampleObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HString& Method, const HTuple& SampleDistance, const HTuple& GenParamName, const HTuple& GenParamValue);

  // Sample a 3D object model.
  HObjectModel3D SampleObjectModel3d(const HString& Method, double SampleDistance, const HString& GenParamName, double GenParamValue) const;

  // Sample a 3D object model.
  HObjectModel3D SampleObjectModel3d(const char* Method, double SampleDistance, const char* GenParamName, double GenParamValue) const;

  // Improve the relative transformations between 3D object models based on  their overlaps.
  static HHomMat3DArray RegisterObjectModel3dGlobal(const HObjectModel3DArray& ObjectModels3D, const HHomMat3DArray& HomMats3D, const HTuple& From, const HTuple& To, const HTuple& GenParamName, const HTuple& GenParamValue, HTuple* Scores);

  // Improve the relative transformations between 3D object models based on  their overlaps.
  HHomMat3DArray RegisterObjectModel3dGlobal(const HHomMat3DArray& HomMats3D, const HString& From, Hlong To, const HString& GenParamName, double GenParamValue, HTuple* Scores) const;

  // Improve the relative transformations between 3D object models based on  their overlaps.
  HHomMat3DArray RegisterObjectModel3dGlobal(const HHomMat3DArray& HomMats3D, const char* From, Hlong To, const char* GenParamName, double GenParamValue, HTuple* Scores) const;

  // Search for a transformation between two 3D object models.
  HPose RegisterObjectModel3dPair(const HObjectModel3D& ObjectModel3D2, const HString& Method, const HTuple& GenParamName, const HTuple& GenParamValue, HTuple* Score) const;

  // Search for a transformation between two 3D object models.
  HPose RegisterObjectModel3dPair(const HObjectModel3D& ObjectModel3D2, const HString& Method, const HString& GenParamName, double GenParamValue, HTuple* Score) const;

  // Search for a transformation between two 3D object models.
  HPose RegisterObjectModel3dPair(const HObjectModel3D& ObjectModel3D2, const char* Method, const char* GenParamName, double GenParamValue, HTuple* Score) const;

  // Create a 3D object model that represents a point cloud from a set of 3D points.
  void GenObjectModel3dFromPoints(const HTuple& X, const HTuple& Y, const HTuple& Z);

  // Create a 3D object model that represents a point cloud from a set of 3D points.
  void GenObjectModel3dFromPoints(double X, double Y, double Z);

  // Create a 3D object model that represents a box.
  static HObjectModel3DArray GenBoxObjectModel3d(const HPoseArray& Pose, const HTuple& LengthX, const HTuple& LengthY, const HTuple& LengthZ);

  // Create a 3D object model that represents a box.
  void GenBoxObjectModel3d(const HPose& Pose, double LengthX, double LengthY, double LengthZ);

  // Create a 3D object model that represents a plane.
  void GenPlaneObjectModel3d(const HPose& Pose, const HTuple& XExtent, const HTuple& YExtent);

  // Create a 3D object model that represents a plane.
  void GenPlaneObjectModel3d(const HPose& Pose, double XExtent, double YExtent);

  // Create a 3D object model that represents a sphere from x,y,z coordinates.
  static HObjectModel3DArray GenSphereObjectModel3dCenter(const HTuple& X, const HTuple& Y, const HTuple& Z, const HTuple& Radius);

  // Create a 3D object model that represents a sphere from x,y,z coordinates.
  void GenSphereObjectModel3dCenter(double X, double Y, double Z, double Radius);

  // Create a 3D object model that represents a sphere.
  static HObjectModel3DArray GenSphereObjectModel3d(const HPoseArray& Pose, const HTuple& Radius);

  // Create a 3D object model that represents a sphere.
  void GenSphereObjectModel3d(const HPose& Pose, double Radius);

  // Create a 3D object model that represents a cylinder.
  static HObjectModel3DArray GenCylinderObjectModel3d(const HPoseArray& Pose, const HTuple& Radius, const HTuple& MinExtent, const HTuple& MaxExtent);

  // Create a 3D object model that represents a cylinder.
  void GenCylinderObjectModel3d(const HPose& Pose, double Radius, double MinExtent, double MaxExtent);

  // Calculate the smallest bounding box around the points of a  3D object model.
  static HPoseArray SmallestBoundingBoxObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HString& Type, HTuple* Length1, HTuple* Length2, HTuple* Length3);

  // Calculate the smallest bounding box around the points of a  3D object model.
  HPose SmallestBoundingBoxObjectModel3d(const HString& Type, double* Length1, double* Length2, double* Length3) const;

  // Calculate the smallest bounding box around the points of a  3D object model.
  HPose SmallestBoundingBoxObjectModel3d(const char* Type, double* Length1, double* Length2, double* Length3) const;

  // Calculate the smallest sphere around the points of a 3D object model.
  static HTuple SmallestSphereObjectModel3d(const HObjectModel3DArray& ObjectModel3D, HTuple* Radius);

  // Calculate the smallest sphere around the points of a 3D object model.
  HTuple SmallestSphereObjectModel3d(double* Radius) const;

  // Intersect a 3D object model with a plane.
  static HObjectModel3DArray IntersectPlaneObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HPoseArray& Plane);

  // Intersect a 3D object model with a plane.
  HObjectModel3D IntersectPlaneObjectModel3d(const HPose& Plane) const;

  // Calculate the convex hull of a 3D object model. 
  static HObjectModel3DArray ConvexHullObjectModel3d(const HObjectModel3DArray& ObjectModel3D);

  // Calculate the convex hull of a 3D object model. 
  HObjectModel3D ConvexHullObjectModel3d() const;

  // Select 3D object models from an array of 3D object models according  to global features.
  static HObjectModel3DArray SelectObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HTuple& Feature, const HString& Operation, const HTuple& MinValue, const HTuple& MaxValue);

  // Select 3D object models from an array of 3D object models according  to global features.
  HObjectModel3D SelectObjectModel3d(const HString& Feature, const HString& Operation, double MinValue, double MaxValue) const;

  // Select 3D object models from an array of 3D object models according  to global features.
  HObjectModel3D SelectObjectModel3d(const char* Feature, const char* Operation, double MinValue, double MaxValue) const;

  // Calculate the area of all faces of a 3D object model.
  static HTuple AreaObjectModel3d(const HObjectModel3DArray& ObjectModel3D);

  // Calculate the area of all faces of a 3D object model.
  double AreaObjectModel3d() const;

  // Calculate the maximal diameter of a 3D object model.
  static HTuple MaxDiameterObjectModel3d(const HObjectModel3DArray& ObjectModel3D);

  // Calculate the maximal diameter of a 3D object model.
  double MaxDiameterObjectModel3d() const;

  // Calculates the mean or the central moment of second order for a 3D object  model.
  static HTuple MomentsObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HTuple& MomentsToCalculate);

  // Calculates the mean or the central moment of second order for a 3D object  model.
  double MomentsObjectModel3d(const HString& MomentsToCalculate) const;

  // Calculates the mean or the central moment of second order for a 3D object  model.
  double MomentsObjectModel3d(const char* MomentsToCalculate) const;

  // Calculate the volume of a 3D object model relative to a plane.
  static HTuple VolumeObjectModel3dRelativeToPlane(const HObjectModel3DArray& ObjectModel3D, const HPoseArray& Plane, const HTuple& Mode, const HTuple& UseFaceOrientation);

  // Calculate the volume of a 3D object model relative to a plane.
  double VolumeObjectModel3dRelativeToPlane(const HPose& Plane, const HString& Mode, const HString& UseFaceOrientation) const;

  // Calculate the volume of a 3D object model relative to a plane.
  double VolumeObjectModel3dRelativeToPlane(const HPose& Plane, const char* Mode, const char* UseFaceOrientation) const;

  // Remove points from a 3D object model by projecting it to a virtual view  and removing all points outside of a given region.
  static HObjectModel3DArray ReduceObjectModel3dByView(const HRegion& Region, const HObjectModel3DArray& ObjectModel3D, const HTuple& CamParam, const HPoseArray& Pose);

  // Remove points from a 3D object model by projecting it to a virtual view  and removing all points outside of a given region.
  HObjectModel3D ReduceObjectModel3dByView(const HRegion& Region, const HTuple& CamParam, const HPose& Pose) const;

  // Determine the connected components of the 3D object model.
  static HObjectModel3DArray ConnectionObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HTuple& Feature, const HTuple& Value);

  // Determine the connected components of the 3D object model.
  HObjectModel3DArray ConnectionObjectModel3d(const HString& Feature, double Value) const;

  // Determine the connected components of the 3D object model.
  HObjectModel3DArray ConnectionObjectModel3d(const char* Feature, double Value) const;

  // Apply a threshold to an attribute of 3D object models.
  static HObjectModel3DArray SelectPointsObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HTuple& Attrib, const HTuple& MinValue, const HTuple& MaxValue);

  // Apply a threshold to an attribute of 3D object models.
  HObjectModel3D SelectPointsObjectModel3d(const HString& Attrib, double MinValue, double MaxValue) const;

  // Apply a threshold to an attribute of 3D object models.
  HObjectModel3D SelectPointsObjectModel3d(const char* Attrib, double MinValue, double MaxValue) const;

  // Get the depth or the index of a displayed 3D object model.
  static HTuple GetDispObjectModel3dInfo(const HWindow& WindowHandle, const HTuple& Row, const HTuple& Column, const HTuple& Information);

  // Get the depth or the index of a displayed 3D object model.
  static Hlong GetDispObjectModel3dInfo(const HWindow& WindowHandle, double Row, double Column, const HString& Information);

  // Get the depth or the index of a displayed 3D object model.
  static Hlong GetDispObjectModel3dInfo(const HWindow& WindowHandle, double Row, double Column, const char* Information);

  // Render 3D object models to get an image.
  static HImage RenderObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HTuple& CamParam, const HPoseArray& Pose, const HTuple& GenParamName, const HTuple& GenParamValue);

  // Render 3D object models to get an image.
  HImage RenderObjectModel3d(const HTuple& CamParam, const HPose& Pose, const HTuple& GenParamName, const HTuple& GenParamValue) const;

  // Display 3D object models.
  static void DispObjectModel3d(const HWindow& WindowHandle, const HObjectModel3DArray& ObjectModel3D, const HTuple& CamParam, const HPoseArray& Pose, const HTuple& GenParamName, const HTuple& GenParamValue);

  // Display 3D object models.
  void DispObjectModel3d(const HWindow& WindowHandle, const HTuple& CamParam, const HPose& Pose, const HTuple& GenParamName, const HTuple& GenParamValue) const;

  // Copy a 3D object model.
  HObjectModel3D CopyObjectModel3d(const HTuple& Attributes) const;

  // Copy a 3D object model.
  HObjectModel3D CopyObjectModel3d(const HString& Attributes) const;

  // Copy a 3D object model.
  HObjectModel3D CopyObjectModel3d(const char* Attributes) const;

  // Prepare a 3D object model for a certain operation.
  static void PrepareObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HString& Purpose, const HString& OverwriteData, const HTuple& GenParamName, const HTuple& GenParamValue);

  // Prepare a 3D object model for a certain operation.
  void PrepareObjectModel3d(const HString& Purpose, const HString& OverwriteData, const HString& GenParamName, const HString& GenParamValue) const;

  // Prepare a 3D object model for a certain operation.
  void PrepareObjectModel3d(const char* Purpose, const char* OverwriteData, const char* GenParamName, const char* GenParamValue) const;

  // Transform 3D points from a 3D object model to images.
  HImage ObjectModel3dToXyz(HImage* Y, HImage* Z, const HString& Type, const HTuple& CamParam, const HPose& Pose) const;

  // Transform 3D points from a 3D object model to images.
  HImage ObjectModel3dToXyz(HImage* Y, HImage* Z, const char* Type, const HTuple& CamParam, const HPose& Pose) const;

  // Transform 3D points from images to a 3D object model.
  void XyzToObjectModel3d(const HImage& X, const HImage& Y, const HImage& Z);

  // Return attributes of 3D object models.
  static HTuple GetObjectModel3dParams(const HObjectModel3DArray& ObjectModel3D, const HTuple& ParamName);

  // Return attributes of 3D object models.
  HTuple GetObjectModel3dParams(const HString& ParamName) const;

  // Return attributes of 3D object models.
  HTuple GetObjectModel3dParams(const char* ParamName) const;

  // Project a 3D object model into image coordinates.
  HXLDCont ProjectObjectModel3d(const HTuple& CamParam, const HPose& Pose, const HTuple& GenParamName, const HTuple& GenParamValue) const;

  // Project a 3D object model into image coordinates.
  HXLDCont ProjectObjectModel3d(const HTuple& CamParam, const HPose& Pose, const HString& GenParamName, const HString& GenParamValue) const;

  // Project a 3D object model into image coordinates.
  HXLDCont ProjectObjectModel3d(const HTuple& CamParam, const HPose& Pose, const char* GenParamName, const char* GenParamValue) const;

  // Apply a rigid 3D transformation to 3D object models.
  static HObjectModel3DArray RigidTransObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HPoseArray& Pose);

  // Apply a rigid 3D transformation to 3D object models.
  HObjectModel3D RigidTransObjectModel3d(const HPose& Pose) const;

  // Apply an arbitrary projective 3D transformation to 3D object models.
  static HObjectModel3DArray ProjectiveTransObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HHomMat3DArray& HomMat3D);

  // Apply an arbitrary projective 3D transformation to 3D object models.
  HObjectModel3D ProjectiveTransObjectModel3d(const HHomMat3D& HomMat3D) const;

  // Apply an arbitrary affine 3D transformation to 3D object models.
  static HObjectModel3DArray AffineTransObjectModel3d(const HObjectModel3DArray& ObjectModel3D, const HHomMat3DArray& HomMat3D);

  // Apply an arbitrary affine 3D transformation to 3D object models.
  HObjectModel3D AffineTransObjectModel3d(const HHomMat3D& HomMat3D) const;

  // Serialize a 3D object model.
  HSerializedItem SerializeObjectModel3d() const;

  // Deserialize a serialized 3D object model.
  void DeserializeObjectModel3d(const HSerializedItem& SerializedItemHandle);

  // Writes a 3D object model to a file.
  void WriteObjectModel3d(const HString& FileType, const HString& FileName, const HTuple& GenParamName, const HTuple& GenParamValue) const;

  // Writes a 3D object model to a file.
  void WriteObjectModel3d(const HString& FileType, const HString& FileName, const HString& GenParamName, const HString& GenParamValue) const;

  // Writes a 3D object model to a file.
  void WriteObjectModel3d(const char* FileType, const char* FileName, const char* GenParamName, const char* GenParamValue) const;

  // Read a 3D object model from a file.
  HTuple ReadObjectModel3d(const HString& FileName, const HTuple& Scale, const HTuple& GenParamName, const HTuple& GenParamValue);

  // Read a 3D object model from a file.
  HString ReadObjectModel3d(const HString& FileName, const HString& Scale, const HString& GenParamName, const HString& GenParamValue);

  // Read a 3D object model from a file.
  HString ReadObjectModel3d(const char* FileName, const char* Scale, const char* GenParamName, const char* GenParamValue);

  // Read a 3D object model from a DXF file.
  HTuple ReadObjectModel3dDxf(const HString& FileName, const HTuple& Scale, const HTuple& GenParamNames, const HTuple& GenParamValues);

  // Read a 3D object model from a DXF file.
  HString ReadObjectModel3dDxf(const HString& FileName, const HString& Scale, const HString& GenParamNames, double GenParamValues);

  // Read a 3D object model from a DXF file.
  HString ReadObjectModel3dDxf(const char* FileName, const char* Scale, const char* GenParamNames, double GenParamValues);

  // Compute the calibrated scene flow between two stereo image pairs.
  static HObjectModel3DArray SceneFlowCalib(const HImage& ImageRect1T1, const HImage& ImageRect2T1, const HImage& ImageRect1T2, const HImage& ImageRect2T2, const HImage& Disparity, const HTuple& SmoothingFlow, const HTuple& SmoothingDisparity, const HTuple& GenParamName, const HTuple& GenParamValue, const HTuple& CamParamRect1, const HTuple& CamParamRect2, const HPose& RelPoseRect);

  // Compute the calibrated scene flow between two stereo image pairs.
  void SceneFlowCalib(const HImage& ImageRect1T1, const HImage& ImageRect2T1, const HImage& ImageRect1T2, const HImage& ImageRect2T2, const HImage& Disparity, double SmoothingFlow, double SmoothingDisparity, const HString& GenParamName, const HString& GenParamValue, const HTuple& CamParamRect1, const HTuple& CamParamRect2, const HPose& RelPoseRect);

  // Compute the calibrated scene flow between two stereo image pairs.
  void SceneFlowCalib(const HImage& ImageRect1T1, const HImage& ImageRect2T1, const HImage& ImageRect1T2, const HImage& ImageRect2T2, const HImage& Disparity, double SmoothingFlow, double SmoothingDisparity, const char* GenParamName, const char* GenParamValue, const HTuple& CamParamRect1, const HTuple& CamParamRect2, const HPose& RelPoseRect);

};

// forward declarations and types for internal array implementation

template<class T> class HSmartPtr;
template<class T> class HToolArrayRef;

typedef HToolArrayRef<HObjectModel3D> HObjectModel3DArrayRef;
typedef HSmartPtr< HObjectModel3DArrayRef > HObjectModel3DArrayPtr;


// Represents multiple tool instances
class LIntExport HObjectModel3DArray : public HToolArray
{

public:

  // Create empty array
  HObjectModel3DArray();

  // Create array from native array of tool instances
  HObjectModel3DArray(HObjectModel3D* classes, Hlong length);

  // Copy constructor
  HObjectModel3DArray(const HObjectModel3DArray &tool_array);

  // Destructor
  virtual ~HObjectModel3DArray();

  // Assignment operator
  HObjectModel3DArray &operator=(const HObjectModel3DArray &tool_array);

  // Clears array and all tool instances
  virtual void Clear();

  // Get array of native tool instances
  const HObjectModel3D* Tools() const;

  // Get number of tools
  virtual Hlong Length() const;

  // Create tool array from tuple of handles
  virtual void SetFromTuple(const HTuple& handles);

  // Get tuple of handles for tool array
  virtual HTuple ConvertToTuple() const;

protected:

// Smart pointer to internal data container
   HObjectModel3DArrayPtr *mArrayPtr;
};

}

#endif
