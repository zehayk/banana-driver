#pragma once
// Bone ingame indexes:
//    head = 6,
//    neck = 5,
//    spine = 4,
//    spine_1 = 2,
//    left_shoulder = 8,
//    left_arm = 9,
//    left_hand = 11,
//    cock = 0,
//    right_shoulder = 13,
//    right_arm = 14,
//    right_hand = 16,
//    left_hip = 22,
//    left_knee = 23,
//    left_feet = 24,
//    right_hip = 25,
//    right_knee = 26,
//    right_feet = 27

enum bone
{
    head,
    neck,
    spine,
    spine_1,
    left_shoulder,
    left_arm,
    left_hand,
    cock,
    right_shoulder,
    right_arm,
    right_hand,
    left_hip,
    left_knee,
    left_feet,
    right_hip,
    right_knee,
    right_feet
};

// must be kept in this exact order, this way we can use 
// `bone_to_game_idx[neck]` to access bone idx without map
int bone_to_game_idx[17] =
{
    6,
    5,
    4,
    2,
    8,
    9,
    11,
    0,
    13,
    14,
    16,
    22,
    23,
    24,
    25,
    26,
    27
};

struct BoneConnection
{
    bone bomeFrom;
    bone boneTo;

    //BoneConnection(int b1, int b2) : bone1(b1), bone2(b2) {}
};

BoneConnection boneConnection[] = {
    BoneConnection(head, neck),   // head to neck
    BoneConnection(neck, spine),   // neck to spine
    BoneConnection(spine, cock),   // spine to hip
    BoneConnection(spine, left_shoulder),   // spine to left shoulder
    BoneConnection(left_shoulder, left_arm),   // left shoulder to left arm
    BoneConnection(left_arm, left_hand),  // left arm to hand
    BoneConnection(spine, right_shoulder),  // spine to right shoulder
    BoneConnection(right_shoulder, right_arm), // right shoulder to arm
    BoneConnection(right_arm, right_hand), // right arm to hand
    BoneConnection(spine, spine_1),   // spine to spine_1
    BoneConnection(cock, left_hip),  // hip to left_hip
    BoneConnection(cock, right_hip),  // hip to right_hip
    BoneConnection(left_hip, left_knee), // left_hip to left_knee
    BoneConnection(left_knee, left_feet), // left_knee to left_foot
    BoneConnection(right_hip, right_knee), // right_hip to right_knee
    BoneConnection(right_knee, right_feet)  // right knee to right foot
};

//BoneConnection boneConnection[] = {
//    BoneConnection(6, 5),   // head to neck
//    BoneConnection(5, 4),   // neck to spine
//    BoneConnection(4, 0),   // spine to hip
//    BoneConnection(4, 8),   // spine to left shoulder
//    BoneConnection(8, 9),   // left shoulder to left arm
//    BoneConnection(9, 11),  // arm to hand
//    BoneConnection(4, 13),  // spine to right shoulder
//    BoneConnection(13, 14), // right shoulder to arm
//    BoneConnection(14, 16), // right arm to hand
//    BoneConnection(4, 2),   // spine to spine_1
//    BoneConnection(0, 22),  // hip to left_hip
//    BoneConnection(0, 25),  // hip to right_hip
//    BoneConnection(22, 23), // left_hip to left_knee
//    BoneConnection(23, 24), // left_knee to left_foot
//    BoneConnection(25, 26), // right_hip to right_knee
//    BoneConnection(26, 27)  // right knee to right foot
//};