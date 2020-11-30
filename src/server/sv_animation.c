//
// sv_animatino.c
//

#include "server.h"
#include "../qcommon/qfiles.h"

typedef enum {
	MOD_BAD,
	MOD_BRUSH,
	MOD_MESH,
	MOD_MDS,
	MOD_MDC // Ridah
} modtype_t;

typedef struct model_s {
	char name[MAX_QPATH];
	modtype_t type;
	int index;                      // model = tr.models[model->index]
	int dataSize;                   // just for listing purposes
	mdsHeader_t *mds;               // only if type == MOD_MDS
} model_t;

model_t models[NUM_MODELS];
float frontlerp, backlerp;
float torsoFrontlerp, torsoBacklerp;
int *boneRefs;
mdsBoneFrame_t bones[MDS_MAX_BONES], rawBones[MDS_MAX_BONES], oldBones[MDS_MAX_BONES];
char validBones[MDS_MAX_BONES];
char newBones[MDS_MAX_BONES];
mdsBoneFrame_t *bonePtr, *bone, *parentBone;
mdsBoneFrameCompressed_t *cBonePtr, *cTBonePtr, *cOldBonePtr, *cOldTBonePtr, *cBoneList, *cOldBoneList, *cBoneListTorso, *cOldBoneListTorso;
mdsBoneInfo_t *boneInfo, *thisBoneInfo, *parentBoneInfo;
mdsFrame_t *frame, *torsoFrame;
mdsFrame_t *oldFrame, *oldTorsoFrame;
int	frameSize;
short *sh, *sh2;
float *pf;
vec3_t angles, tangles, torsoParentOffset, torsoAxis[3], tmpAxis[3];
vec3_t vec, v2, dir;
float diff, a1, a2;
qboolean isTorso, fullTorso;
vec4_t m1[4], m2[4];
vec3_t t;
clientAnimationInfo_t lastAnimInfo;
float LAVangle;
float sp, sy, cp, cy;

ID_INLINE void LocalVectorMA(vec3_t org, float dist, vec3_t vec, vec3_t out) {
	out[0] = org[0] + dist * vec[0];
	out[1] = org[1] + dist * vec[1];
	out[2] = org[2] + dist * vec[2];
}

ID_INLINE void SLerp_Normal(vec3_t from, vec3_t to, float tt, vec3_t out) {
	float ft = 1.0 - tt;

	out[0] = from[0] * ft + to[0] * tt;
	out[1] = from[1] * ft + to[1] * tt;
	out[2] = from[2] * ft + to[2] * tt;

	VectorNormalize(out);
}

ID_INLINE void LocalAngleVector(vec3_t angles, vec3_t forward) {
	LAVangle = angles[YAW] * (M_PI * 2 / 360);
	sy = sin(LAVangle);
	cy = cos(LAVangle);
	LAVangle = angles[PITCH] * (M_PI * 2 / 360);
	sp = sin(LAVangle);
	cp = cos(LAVangle);

	forward[0] = cp * cy;
	forward[1] = cp * sy;
	forward[2] = -sp;
}

// can put an axis rotation followed by a translation directly into one matrix
// TTimo: const usage would require an explicit cast, non ANSI C
// see unix/const-arg.c
ID_INLINE void Matrix4FromAxisPlusTranslation( /*const*/ vec3_t axis[3], const vec3_t t, vec4_t dst[4]) {
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			dst[i][j] = axis[i][j];
		}
		dst[3][i] = 0;
		dst[i][3] = t[i];
	}
	dst[3][3] = 1;
}

// can put a scaled axis rotation followed by a translation directly into one matrix
// TTimo: const usage would require an explicit cast, non ANSI C
// see unix/const-arg.c
ID_INLINE void Matrix4FromScaledAxisPlusTranslation( /*const*/ vec3_t axis[3], const float scale, const vec3_t t, vec4_t dst[4]) {
	int i, j;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			dst[i][j] = scale * axis[i][j];
			if (i == j) {
				dst[i][j] += 1.0f - scale;
			}
		}
		dst[3][i] = 0;
		dst[i][3] = t[i];
	}
	dst[3][3] = 1;
}

// TTimo: const usage would require an explicit cast, non ANSI C
// see unix/const-arg.c
ID_INLINE void Matrix4MultiplyInto3x3AndTranslation( /*const*/ vec4_t a[4], /*const*/ vec4_t b[4], vec3_t dst[3], vec3_t t) {
	dst[0][0] = a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0] + a[0][3] * b[3][0];
	dst[0][1] = a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1] + a[0][3] * b[3][1];
	dst[0][2] = a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2] + a[0][3] * b[3][2];
	t[0] = a[0][0] * b[0][3] + a[0][1] * b[1][3] + a[0][2] * b[2][3] + a[0][3] * b[3][3];

	dst[1][0] = a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0] + a[1][3] * b[3][0];
	dst[1][1] = a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1] + a[1][3] * b[3][1];
	dst[1][2] = a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2] + a[1][3] * b[3][2];
	t[1] = a[1][0] * b[0][3] + a[1][1] * b[1][3] + a[1][2] * b[2][3] + a[1][3] * b[3][3];

	dst[2][0] = a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0] + a[2][3] * b[3][0];
	dst[2][1] = a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1] + a[2][3] * b[3][1];
	dst[2][2] = a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2] + a[2][3] * b[3][2];
	t[2] = a[2][0] * b[0][3] + a[2][1] * b[1][3] + a[2][2] * b[2][3] + a[2][3] * b[3][3];
}

ID_INLINE void LocalScaledMatrixTransformVector(vec3_t in, float s, vec3_t mat[3], vec3_t out) {
	out[0] = (1.0f - s) * in[0] + s * (in[0] * mat[0][0] + in[1] * mat[0][1] + in[2] * mat[0][2]);
	out[1] = (1.0f - s) * in[1] + s * (in[0] * mat[1][0] + in[1] * mat[1][1] + in[2] * mat[1][2]);
	out[2] = (1.0f - s) * in[2] + s * (in[0] * mat[2][0] + in[1] * mat[2][1] + in[2] * mat[2][2]);
}

ID_INLINE void Matrix3Transpose(vec3_t matrix[3], vec3_t transpose[3]) {
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			transpose[i][j] = matrix[j][i];
		}
	}
}

model_t *SV_GetModelByHandle(qhandle_t index) {
	model_t     *mod;

	// out of range gets the defualt model
	if (index < 1 || index >= NUM_MODELS) {
		return &models[0];
	}

	mod = &models[index];

	return mod;
}

/*
===============
SV_RecursiveBoneListAdd
===============
*/
void SV_RecursiveBoneListAdd(int bi, int *boneList, int *numBones, mdsBoneInfo_t *boneInfoList) {
	if (boneInfoList[bi].parent >= 0) {
		SV_RecursiveBoneListAdd(boneInfoList[bi].parent, boneList, numBones, boneInfoList);

	}

	boneList[(*numBones)++] = bi;
}

/*
==============
SV_CalcBoneLerp
==============
*/
void SV_CalcBoneLerp(mdsHeader_t *header, clientAnimationInfo_t *animInfo, int boneNum) {
	int j;

	if (!animInfo || !header || boneNum < 0 || boneNum >= MDS_MAX_BONES) {
		return;
	}

	thisBoneInfo = &boneInfo[boneNum];

	if (!thisBoneInfo) {
		return;
	}

	if (thisBoneInfo->parent >= 0) {
		parentBone = &bones[thisBoneInfo->parent];
		parentBoneInfo = &boneInfo[thisBoneInfo->parent];
	}
	else {
		parentBone = NULL;
		parentBoneInfo = NULL;
	}

	if (thisBoneInfo->torsoWeight) {
		cTBonePtr = &cBoneListTorso[boneNum];
		cOldTBonePtr = &cOldBoneListTorso[boneNum];
		isTorso = qtrue;
		if (thisBoneInfo->torsoWeight == 1.0f) {
			fullTorso = qtrue;
		}
	}
	else {
		isTorso = qfalse;
		fullTorso = qfalse;
	}
	cBonePtr = &cBoneList[boneNum];
	cOldBonePtr = &cOldBoneList[boneNum];

	bonePtr = &bones[boneNum];

	newBones[boneNum] = 1;

	// rotation (take into account 170 to -170 lerps, which need to take the shortest route)
	if (fullTorso) {
		sh = (short *)cTBonePtr->angles;
		sh2 = (short *)cOldTBonePtr->angles;
		pf = angles;

		a1 = SHORT2ANGLE(*(sh++)); a2 = SHORT2ANGLE(*(sh2++)); diff = AngleNormalize180(a1 - a2);
		*(pf++) = a1 - torsoBacklerp * diff;
		a1 = SHORT2ANGLE(*(sh++)); a2 = SHORT2ANGLE(*(sh2++)); diff = AngleNormalize180(a1 - a2);
		*(pf++) = a1 - torsoBacklerp * diff;
		a1 = SHORT2ANGLE(*(sh++)); a2 = SHORT2ANGLE(*(sh2++)); diff = AngleNormalize180(a1 - a2);
		*(pf++) = a1 - torsoBacklerp * diff;
	}
	else {
		sh = (short *)cBonePtr->angles;
		sh2 = (short *)cOldBonePtr->angles;
		pf = angles;

		a1 = SHORT2ANGLE(*(sh++)); a2 = SHORT2ANGLE(*(sh2++)); diff = AngleNormalize180(a1 - a2);
		*(pf++) = a1 - backlerp * diff;
		a1 = SHORT2ANGLE(*(sh++)); a2 = SHORT2ANGLE(*(sh2++)); diff = AngleNormalize180(a1 - a2);
		*(pf++) = a1 - backlerp * diff;
		a1 = SHORT2ANGLE(*(sh++)); a2 = SHORT2ANGLE(*(sh2++)); diff = AngleNormalize180(a1 - a2);
		*(pf++) = a1 - backlerp * diff;

		if (isTorso) {

			sh = (short *)cTBonePtr->angles;
			sh2 = (short *)cOldTBonePtr->angles;
			pf = tangles;

			a1 = SHORT2ANGLE(*(sh++)); a2 = SHORT2ANGLE(*(sh2++)); diff = AngleNormalize180(a1 - a2);
			*(pf++) = a1 - torsoBacklerp * diff;
			a1 = SHORT2ANGLE(*(sh++)); a2 = SHORT2ANGLE(*(sh2++)); diff = AngleNormalize180(a1 - a2);
			*(pf++) = a1 - torsoBacklerp * diff;
			a1 = SHORT2ANGLE(*(sh++)); a2 = SHORT2ANGLE(*(sh2++)); diff = AngleNormalize180(a1 - a2);
			*(pf++) = a1 - torsoBacklerp * diff;

			// blend the angles together
			for (j = 0; j < 3; j++) {
				diff = tangles[j] - angles[j];
				if (fabs(diff) > 180) {
					diff = AngleNormalize180(diff);
				}
				angles[j] = angles[j] + thisBoneInfo->torsoWeight * diff;
			}

		}
	}

	AnglesToAxis(angles, bonePtr->matrix);

	if (parentBone) {
		if (fullTorso) {
			sh = (short *)cTBonePtr->ofsAngles;
			sh2 = (short *)cOldTBonePtr->ofsAngles;
		}
		else {
			sh = (short *)cBonePtr->ofsAngles;
			sh2 = (short *)cOldBonePtr->ofsAngles;
		}

		pf = angles;
		*(pf++) = SHORT2ANGLE(*(sh++));
		*(pf++) = SHORT2ANGLE(*(sh++));
		*(pf++) = 0;
		LocalAngleVector(angles, v2);     // new

		pf = angles;
		*(pf++) = SHORT2ANGLE(*(sh2++));
		*(pf++) = SHORT2ANGLE(*(sh2++));
		*(pf++) = 0;
		LocalAngleVector(angles, vec);    // old

										  // blend the angles together
		if (fullTorso) {
			SLerp_Normal(vec, v2, torsoFrontlerp, dir);
		}
		else {
			SLerp_Normal(vec, v2, frontlerp, dir);
		}

		// translation
		if (!fullTorso && isTorso) {    // partial legs/torso, need to lerp according to torsoWeight

										// calc the torso frame
			sh = (short *)cTBonePtr->ofsAngles;
			sh2 = (short *)cOldTBonePtr->ofsAngles;

			pf = angles;
			*(pf++) = SHORT2ANGLE(*(sh++));
			*(pf++) = SHORT2ANGLE(*(sh++));
			*(pf++) = 0;
			LocalAngleVector(angles, v2);     // new

			pf = angles;
			*(pf++) = SHORT2ANGLE(*(sh2++));
			*(pf++) = SHORT2ANGLE(*(sh2++));
			*(pf++) = 0;
			LocalAngleVector(angles, vec);    // old

											  // blend the angles together
			SLerp_Normal(vec, v2, torsoFrontlerp, v2);

			// blend the torso/legs together
			SLerp_Normal(dir, v2, thisBoneInfo->torsoWeight, dir);

		}

		LocalVectorMA(parentBone->translation, thisBoneInfo->parentDist, dir, bonePtr->translation);
	}
	else {    // just interpolate the frame positions
		bonePtr->translation[0] = frontlerp * frame->parentOffset[0] + backlerp * oldFrame->parentOffset[0];
		bonePtr->translation[1] = frontlerp * frame->parentOffset[1] + backlerp * oldFrame->parentOffset[1];
		bonePtr->translation[2] = frontlerp * frame->parentOffset[2] + backlerp * oldFrame->parentOffset[2];
	}
	//
	if (boneNum == header->torsoParent) { // this is the torsoParent
		VectorCopy(bonePtr->translation, torsoParentOffset);
	}
	validBones[boneNum] = 1;
	//
	rawBones[boneNum] = *bonePtr;
	newBones[boneNum] = 1;
}


/*
==============
SV_CalcBone
==============
*/
void SV_CalcBone(mdsHeader_t *header, clientAnimationInfo_t *animInfo, int boneNum) {
	int j;

	thisBoneInfo = &boneInfo[boneNum];
	if (thisBoneInfo->torsoWeight) {
		cTBonePtr = &cBoneListTorso[boneNum];
		isTorso = qtrue;
		if (thisBoneInfo->torsoWeight == 1.0f) {
			fullTorso = qtrue;
		}
	}
	else {
		isTorso = qfalse;
		fullTorso = qfalse;
	}
	cBonePtr = &cBoneList[boneNum];

	bonePtr = &bones[boneNum];

	// we can assume the parent has already been uncompressed for this frame + lerp
	if (thisBoneInfo->parent >= 0) {
		parentBone = &bones[thisBoneInfo->parent];
		parentBoneInfo = &boneInfo[thisBoneInfo->parent];
	}
	else {
		parentBone = NULL;
		parentBoneInfo = NULL;
	}

	// rotation
	if (fullTorso) {
		VectorCopy(cTBonePtr->angles, angles);
	}
	else {
		VectorCopy(cBonePtr->angles, angles);
		if (isTorso) {
			VectorCopy(cTBonePtr->angles, tangles);
			// blend the angles together
			for (j = 0; j < 3; j++) {
				diff = tangles[j] - angles[j];
				if (fabs(diff) > 180) {
					diff = AngleNormalize180(diff);
				}
				angles[j] = angles[j] + thisBoneInfo->torsoWeight * diff;
			}
		}
	}
	AnglesToAxis(angles, bonePtr->matrix);

	// translation
	if (parentBone) {

		if (fullTorso) {
			angles[0] = cTBonePtr->ofsAngles[0];
			angles[1] = cTBonePtr->ofsAngles[1];
			angles[2] = 0;
			LocalAngleVector(angles, vec);
			LocalVectorMA(parentBone->translation, thisBoneInfo->parentDist, vec, bonePtr->translation);
		}
		else {

			angles[0] = cBonePtr->ofsAngles[0];
			angles[1] = cBonePtr->ofsAngles[1];
			angles[2] = 0;
			LocalAngleVector(angles, vec);

			if (isTorso) {
				tangles[0] = cTBonePtr->ofsAngles[0];
				tangles[1] = cTBonePtr->ofsAngles[1];
				tangles[2] = 0;
				LocalAngleVector(tangles, v2);

				// blend the angles together
				SLerp_Normal(vec, v2, thisBoneInfo->torsoWeight, vec);
				LocalVectorMA(parentBone->translation, thisBoneInfo->parentDist, vec, bonePtr->translation);

			}
			else {    // legs bone
				LocalVectorMA(parentBone->translation, thisBoneInfo->parentDist, vec, bonePtr->translation);
			}
		}
	}
	else {    // just use the frame position
		bonePtr->translation[0] = frame->parentOffset[0];
		bonePtr->translation[1] = frame->parentOffset[1];
		bonePtr->translation[2] = frame->parentOffset[2];
	}
	//
	if (boneNum == header->torsoParent) { // this is the torsoParent
		VectorCopy(bonePtr->translation, torsoParentOffset);
	}
	//
	validBones[boneNum] = 1;
	//
	rawBones[boneNum] = *bonePtr;
	newBones[boneNum] = 1;
}

/*
==============
SV_CalcBones

The list of bones[] should only be built and modified from within here
==============
*/
void SV_CalcBones(mdsHeader_t *header, clientAnimationInfo_t *animInfo, int *boneList, int numBones) {
	int i;
	int     *boneRefs;
	float torsoWeight;

	//
	// if the entity has changed since the last time the bones were built, reset them
	//
	if (memcmp(&lastAnimInfo, animInfo, sizeof(clientAnimationInfo_t))) {
		// different, cached bones are not valid
		memset(validBones, 0, header->numBones);
		lastAnimInfo = *animInfo;
	}

	memset(newBones, 0, header->numBones);

	backlerp = 0;
	frontlerp = 1;

	torsoBacklerp = 0;
	torsoFrontlerp = 1;

	frameSize = (int)(sizeof(mdsFrame_t) + (header->numBones - 1) * sizeof(mdsBoneFrameCompressed_t));

	frame = (mdsFrame_t *)((byte *)header + header->ofsFrames +
		animInfo->legsFrame * frameSize);
	torsoFrame = (mdsFrame_t *)((byte *)header + header->ofsFrames +
		animInfo->torsoFrame * frameSize);
	oldFrame = (mdsFrame_t *)((byte *)header + header->ofsFrames +
		animInfo->legsFrame * frameSize);
	oldTorsoFrame = (mdsFrame_t *)((byte *)header + header->ofsFrames +
		animInfo->torsoFrame * frameSize);

	//
	// lerp all the needed bones (torsoParent is always the first bone in the list)
	//
	cBoneList = frame->bones;
	cBoneListTorso = torsoFrame->bones;

	boneInfo = (mdsBoneInfo_t *)((byte *)header + header->ofsBones);
	boneRefs = boneList;
	//
	Matrix3Transpose(animInfo->torsoAxis, torsoAxis);

	if (qfalse && !backlerp && !torsoBacklerp) {
		for (i = 0; i < numBones; i++, boneRefs++) {

			if (validBones[*boneRefs]) {
				// this bone is still in the cache
				bones[*boneRefs] = rawBones[*boneRefs];
				continue;
			}

			// find our parent, and make sure it has been calculated
			if ((boneInfo[*boneRefs].parent >= 0) && (!validBones[boneInfo[*boneRefs].parent] && !newBones[boneInfo[*boneRefs].parent])) {
				SV_CalcBone(header, animInfo, boneInfo[*boneRefs].parent);
			}

			SV_CalcBone(header, animInfo, *boneRefs);
		}
	}
	else {    // interpolated
		cOldBoneList = oldFrame->bones;
		cOldBoneListTorso = oldTorsoFrame->bones;

		for (i = 0; i < numBones; i++, boneRefs++) {
			if (validBones[*boneRefs]) {
				// this bone is still in the cache
				bones[*boneRefs] = rawBones[*boneRefs];
				continue;
			}

			// find our parent, and make sure it has been calculated
			if ((boneInfo[*boneRefs].parent >= 0) && (!validBones[boneInfo[*boneRefs].parent] && !newBones[boneInfo[*boneRefs].parent])) {
				SV_CalcBoneLerp(header, animInfo, boneInfo[*boneRefs].parent);
			}

			SV_CalcBoneLerp(header, animInfo, *boneRefs);
		}
	}

	// adjust for torso rotations
	torsoWeight = 0;
	boneRefs = boneList;
	for (i = 0; i < numBones; i++, boneRefs++) {
		thisBoneInfo = &boneInfo[*boneRefs];
		bonePtr = &bones[*boneRefs];
		// add torso rotation
		if (thisBoneInfo->torsoWeight > 0) {

			if (!newBones[*boneRefs]) {
				// just copy it back from the previous calc
				bones[*boneRefs] = oldBones[*boneRefs];
				continue;
			}

			if (!(thisBoneInfo->flags & BONEFLAG_TAG)) {
				// 1st multiply with the bone->matrix
				// 2nd translation for rotation relative to bone around torso parent offset
				VectorSubtract(bonePtr->translation, torsoParentOffset, t);
				Matrix4FromAxisPlusTranslation(bonePtr->matrix, t, m1);
				// 3rd scaled rotation
				// 4th translate back to torso parent offset
				// use previously created matrix if available for the same weight
				if (torsoWeight != thisBoneInfo->torsoWeight) {
					Matrix4FromScaledAxisPlusTranslation(torsoAxis, thisBoneInfo->torsoWeight, torsoParentOffset, m2);
					torsoWeight = thisBoneInfo->torsoWeight;
				}
				// multiply matrices to create one matrix to do all calculations
				Matrix4MultiplyInto3x3AndTranslation(m2, m1, bonePtr->matrix, bonePtr->translation);
			}
			else {    // tag's require special handling
					  // rotate each of the axis by the torsoAngles
				LocalScaledMatrixTransformVector(bonePtr->matrix[0], thisBoneInfo->torsoWeight, torsoAxis, tmpAxis[0]);
				LocalScaledMatrixTransformVector(bonePtr->matrix[1], thisBoneInfo->torsoWeight, torsoAxis, tmpAxis[1]);
				LocalScaledMatrixTransformVector(bonePtr->matrix[2], thisBoneInfo->torsoWeight, torsoAxis, tmpAxis[2]);
				memcpy(bonePtr->matrix, tmpAxis, sizeof(tmpAxis));

				// rotate the translation around the torsoParent
				VectorSubtract(bonePtr->translation, torsoParentOffset, t);
				LocalScaledMatrixTransformVector(t, thisBoneInfo->torsoWeight, torsoAxis, bonePtr->translation);
				VectorAdd(bonePtr->translation, torsoParentOffset, bonePtr->translation);
			}
		}
	}

	// backup the final bones
	memcpy(oldBones, bones, sizeof(bones[0]) * header->numBones);
}

/*
===============
SV_GetBoneTag
===============
*/
int SV_GetBoneTag(orientation_t *outTag, mdsHeader_t *mds, int startTagIndex, clientAnimationInfo_t *animInfo, const char *tagName) {
	int i;
	mdsTag_t    *pTag;
	mdsBoneInfo_t *boneInfoList;
	int boneList[MDS_MAX_BONES];
	int numBones;

	if (startTagIndex > mds->numTags) {
		memset(outTag, 0, sizeof(*outTag));
		return -1;
	}

	// find the correct tag
	pTag = (mdsTag_t *)((byte *)mds + mds->ofsTags);

	pTag += startTagIndex;

	for (i = startTagIndex; i < mds->numTags; i++, pTag++) {
		if (!strcmp(pTag->name, tagName)) {
			break;
		}
	}

	if (i >= mds->numTags) {
		memset(outTag, 0, sizeof(*outTag));
		return -1;
	}

	// now build the list of bones we need to calc to get this tag's bone information
	boneInfoList = (mdsBoneInfo_t *)((byte *)mds + mds->ofsBones);
	numBones = 0;

	SV_RecursiveBoneListAdd(pTag->boneIndex, boneList, &numBones, boneInfoList);

	// calc the bones
	SV_CalcBones((mdsHeader_t *)mds, animInfo, boneList, numBones);

	// now extract the orientation for the bone that represents our tag
	memcpy(outTag->axis, bones[pTag->boneIndex].matrix, sizeof(outTag->axis));
	VectorCopy(bones[pTag->boneIndex].translation, outTag->origin);

	return i;
}


int SV_LerpTag(orientation_t *tag, clientAnimationInfo_t *animInfo, char *tagname) {
	model_t *model;
	int retval;
	qhandle_t handle;

	handle = animInfo->bodyModelHandle;

	model = SV_GetModelByHandle(handle);
	if (!model->mds) {
		AxisClear(tag->axis);
		VectorClear(tag->origin);
		return -1;
	}

	if (model->type == MOD_MDS) {    // use bone lerping
		retval = SV_GetBoneTag(tag, model->mds, 0, animInfo, tagname);

		if (retval >= 0) {
			return retval;
		}

		// failed
		return -1;

	}

	return -1;
}

/*
=================
SV_LoadMDS
=================
*/
qboolean SV_LoadMDS(int modelIndex, const char *mod_name) {
	int i, j, k;
	mdsHeader_t         *pinmodel, *mds;
	mdsFrame_t          *frame;
	mdsSurface_t        *surf;
	mdsTriangle_t       *tri;
	mdsVertex_t         *v;
	mdsBoneInfo_t       *bi;
	mdsTag_t            *tag;
	model_t				*mod;
	unsigned			*buffer;
	int					version;
	int					size;
	int					frameSize;
	int                 *collapseMap, *boneref;

#define LL( x ) x = LittleLong( x )

	mod = &models[modelIndex];
	mod->index = modelIndex;

	FS_ReadFile(mod_name, (void **)&buffer);
	pinmodel = (mdsHeader_t *)buffer;

	version = LittleLong(pinmodel->version);
	if (version != MDS_VERSION) {
		FS_FreeFile(buffer);
		return qfalse;
	}

	mod->type = MOD_MDS;
	size = LittleLong(pinmodel->ofsEnd);
	mod->dataSize += size;
	mds = mod->mds = malloc(size);

	memcpy(mds, buffer, LittleLong(pinmodel->ofsEnd));

	FS_FreeFile(buffer);

	LL(mds->ident);
	LL(mds->version);
	LL(mds->numFrames);
	LL(mds->numBones);
	LL(mds->numTags);
	LL(mds->numSurfaces);
	LL(mds->ofsFrames);
	LL(mds->ofsBones);
	LL(mds->ofsTags);
	LL(mds->ofsEnd);
	LL(mds->ofsSurfaces);
	mds->lodBias = LittleFloat(mds->lodBias);
	mds->lodScale = LittleFloat(mds->lodScale);
	LL(mds->torsoParent);

	if (mds->numFrames < 1) {
		return qfalse;
	}

	if (LittleLong(1) != 1) {
		// swap all the frames
		//frameSize = (int)( &((mdsFrame_t *)0)->bones[ mds->numBones ] );
		frameSize = (int)(sizeof(mdsFrame_t) - sizeof(mdsBoneFrameCompressed_t) + mds->numBones * sizeof(mdsBoneFrameCompressed_t));
		for (i = 0; i < mds->numFrames; i++, frame++) {
			frame = (mdsFrame_t *)((byte *)mds + mds->ofsFrames + i * frameSize);
			frame->radius = LittleFloat(frame->radius);
			for (j = 0; j < 3; j++) {
				frame->bounds[0][j] = LittleFloat(frame->bounds[0][j]);
				frame->bounds[1][j] = LittleFloat(frame->bounds[1][j]);
				frame->localOrigin[j] = LittleFloat(frame->localOrigin[j]);
				frame->parentOffset[j] = LittleFloat(frame->parentOffset[j]);
			}
			for (j = 0; j < mds->numBones * sizeof(mdsBoneFrameCompressed_t) / sizeof(short); j++) {
				((short *)frame->bones)[j] = LittleShort(((short *)frame->bones)[j]);
			}
		}

		// swap all the tags
		tag = (mdsTag_t *)((byte *)mds + mds->ofsTags);
		for (i = 0; i < mds->numTags; i++, tag++) {
			LL(tag->boneIndex);
			tag->torsoWeight = LittleFloat(tag->torsoWeight);
		}

		// swap all the bones
		for (i = 0; i < mds->numBones; i++, bi++) {
			bi = (mdsBoneInfo_t *)((byte *)mds + mds->ofsBones + i * sizeof(mdsBoneInfo_t));
			LL(bi->parent);
			bi->torsoWeight = LittleFloat(bi->torsoWeight);
			bi->parentDist = LittleFloat(bi->parentDist);
			LL(bi->flags);
		}
	}

	// swap all the surfaces
	surf = (mdsSurface_t *)((byte *)mds + mds->ofsSurfaces);
	for (i = 0; i < mds->numSurfaces; i++) {
		if (LittleLong(1) != 1) {
			LL(surf->ident);
			LL(surf->shaderIndex);
			LL(surf->minLod);
			LL(surf->ofsHeader);
			LL(surf->ofsCollapseMap);
			LL(surf->numTriangles);
			LL(surf->ofsTriangles);
			LL(surf->numVerts);
			LL(surf->ofsVerts);
			LL(surf->numBoneReferences);
			LL(surf->ofsBoneReferences);
			LL(surf->ofsEnd);
		}

		if (surf->numVerts > SHADER_MAX_VERTEXES) {
			Com_Error(ERR_FATAL, va("R_LoadMDS: %s has more than %i verts on a surface (%i)",
				mod_name, SHADER_MAX_VERTEXES, surf->numVerts));
		}
		if (surf->numTriangles * 3 > SHADER_MAX_INDEXES) {
			Com_Error(ERR_FATAL, va("R_LoadMDS: %s has more than %i triangles on a surface (%i)",
				mod_name, SHADER_MAX_INDEXES / 3, surf->numTriangles));
		}

		if (LittleLong(1) != 1) {
			// swap all the triangles
			tri = (mdsTriangle_t *)((byte *)surf + surf->ofsTriangles);
			for (j = 0; j < surf->numTriangles; j++, tri++) {
				LL(tri->indexes[0]);
				LL(tri->indexes[1]);
				LL(tri->indexes[2]);
			}

			// swap all the vertexes
			v = (mdsVertex_t *)((byte *)surf + surf->ofsVerts);
			for (j = 0; j < surf->numVerts; j++) {
				v->normal[0] = LittleFloat(v->normal[0]);
				v->normal[1] = LittleFloat(v->normal[1]);
				v->normal[2] = LittleFloat(v->normal[2]);

				v->texCoords[0] = LittleFloat(v->texCoords[0]);
				v->texCoords[1] = LittleFloat(v->texCoords[1]);

				v->numWeights = LittleLong(v->numWeights);

				for (k = 0; k < v->numWeights; k++) {
					v->weights[k].boneIndex = LittleLong(v->weights[k].boneIndex);
					v->weights[k].boneWeight = LittleFloat(v->weights[k].boneWeight);
					v->weights[k].offset[0] = LittleFloat(v->weights[k].offset[0]);
					v->weights[k].offset[1] = LittleFloat(v->weights[k].offset[1]);
					v->weights[k].offset[2] = LittleFloat(v->weights[k].offset[2]);
				}

				// find the fixedParent for this vert (if exists)
				v->fixedParent = -1;
				if (v->numWeights == 2) {
					// find the closest parent
					if (VectorLength(v->weights[0].offset) < VectorLength(v->weights[1].offset)) {
						v->fixedParent = 0;
					}
					else {
						v->fixedParent = 1;
					}
					v->fixedDist = VectorLength(v->weights[v->fixedParent].offset);
				}

				v = (mdsVertex_t *)&v->weights[v->numWeights];
			}

			// swap the collapse map
			collapseMap = (int *)((byte *)surf + surf->ofsCollapseMap);
			for (j = 0; j < surf->numVerts; j++, collapseMap++) {
				*collapseMap = LittleLong(*collapseMap);
			}

			// swap the bone references
			boneref = (int *)((byte *)surf + surf->ofsBoneReferences);
			for (j = 0; j < surf->numBoneReferences; j++, boneref++) {
				*boneref = LittleLong(*boneref);
			}
		}

		// find the next surface
		surf = (mdsSurface_t *)((byte *)surf + surf->ofsEnd);
	}

	return qtrue;
}