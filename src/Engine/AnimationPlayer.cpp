#include "AnimationPlayer.h"

#include <iostream>
#include <fstream>
#include <filesystem>

#include <JSON/json.h>

namespace Mega
{
    void Animation::AddFrame(const Frame& in_frame)
    {
        for (uint32_t i = 0; i < frames.size(); ++i)
        {
            if (frames[i].index <= in_frame.index) // Find sorted spot for in_frame
            {
                frames.insert(frames.begin() + i, in_frame);
                return;
            }
        }

        frames.push_back(in_frame);
    }

    void AnimationPlayer::Update(const float in_dt)
    {
        if (!m_playing) { return; }

        // Update timer
        m_tic += in_dt;

        // Get infro from current animation
        FRAME_INDEX_T frameCount = m_pActiveAnimation->GetFrameCount();
        float fps = m_pActiveAnimation->fps;

        // Get new frame index and update (if necessary)
        FRAME_INDEX_T newIndex = (int)(m_tic / ((1 / fps) * 1000)) % frameCount;
        MEGA_ASSERT(newIndex <= m_pActiveAnimation->GetFrameCount(), "Frame index in animation player is overflowing");
        if (m_frameIndex != newIndex)
        {
            m_frameIndex = newIndex;
            const Animation::Frame& frame = m_pActiveAnimation->frames[m_frameIndex];

            // Set new sprite info using using that frame
            m_sprite.coords = frame.coords;
            m_sprite.swatch = frame.swatch;
        }
    }

    bool AnimationPlayer::PlayAnimation(const std::string& in_name)
    {
        // Early exit if just replaying current animation
        if (m_pActiveAnimation && (in_name == m_pActiveAnimation->name)) { return true; }

        // Reset timers
        m_tic = 0.0f;

        for (const auto& anim : m_animations)
        {
            if (anim.name == in_name) {
                SetActiveAnimation(&anim);

                // Set first frame
                const Animation::Frame& frame = m_pActiveAnimation->frames[0];
                m_sprite.coords = frame.coords;
                m_sprite.swatch = frame.swatch;

                return true;
            }
        }

        MEGA_ASSERT(false, "ERROR: Animation could not be found or is not loaded in");
        return false;
    }

    void AnimationPlayer::Play() { m_playing = true; }
    void AnimationPlayer::Pause() { m_playing = false; }

    bool AnimationPlayer::LoadAnimations(const std::string& in_dirPath)
    {
        MEGA_ASSERT(Texture::IsLoaded(m_spriteSheet), "Loading animation before texture is loaded");
        //for (const auto& entry : std::filesystem::directory_iterator(in_folderPath)) // For each file/dir in the animation folder
        //{
        //	std::string fileName = entry.path().filename().string();
        //	std::string fileType = entry.path().extension().string();
        //	std::cout << fileName << std::endl;
        //}

        // Load in Data.json
        std::ifstream fileData(in_dirPath + "/Data.json");
        MEGA_ASSERT(fileData, "ERROR: Animation path could not be found");

        nlohmann::json jsonData;
        fileData >> jsonData;

        Vec2F size = Vec2F(m_spriteSheet.width, m_spriteSheet.height); // Vec2(1566, 1431); used for normalizing frame coords
        size.x = jsonData["meta"]["size"]["w"];
        size.y = jsonData["meta"]["size"]["h"];

        // for (auto& item : jsonData["meta"]["size"]["w"].items())
        // {
        //     char c = item.key().c_str()[0];
        //     switch (c)
        //     {
        //     case ('x'):
        //         size.x = item.value();
        //         break;
        //     case ('y'):
        //         size.y = item.value();
        //         break;
        //     default:
        //         assert("You fucked something up");
        //         continue;
        //     }
        // }

        // Create Animations from json file
        for (auto& f : jsonData["frames"])
        {
            // Get name and number of animation/frame
            std::string frameName = f["filename"];

            uint32_t frameNameLength = 0; // Length of raw name without "01" or ".png"
            for (int i = 0; i < frameName.length(); ++i)
            {
                if (isdigit(frameName[i]) || frameName[i] == '.')
                {
                    frameNameLength = i;
                    break;
                };
            }

            std::string frameNumberString = "0"; // string of the number of the frame, like "01"
            for (int i = 0; i < frameName.length(); ++i)
            {
                if (isdigit(frameName[i]))
                {
                    frameNumberString += frameName[i];
                };
            }


            std::string animationName = frameName.substr(0, frameNameLength);

            int frameNum = std::stoi(frameNumberString);
            {
                // Get frame data
                Animation::Frame frame;
                frame.index = frameNum;
                for (auto& item : f["frame"].items())
                {
                    char c = item.key().c_str()[0];
                    switch (c)
                    {
                    case 'x':
                        frame.coords.x = item.value() / size.x;
                        break;
                    case 'y':
                        frame.coords.y = item.value() / size.y;
                        break;
                    case 'w':
                        frame.swatch.x = item.value() / size.x;
                        break;
                    case 'h':
                        frame.swatch.y = item.value() / size.y;
                        break;
                    default:
                        MEGA_ASSERT(true, "You fucked something up");
                        break;
                    }
                }

                // If animation is already loaded, fill in the frame
                bool isCurrentAnim = false;
                for (auto& anim : m_animations)
                {
                    if (anim.name == animationName)
                    {
                        // Fill into animation and break
                        isCurrentAnim = true;
                        anim.AddFrame(frame);

                        break;
                    }
                }
                // If animation wasn't already loaded, create it and add frame
                if (!isCurrentAnim)
                {
                    // Creating animation
                    Animation newAnimation;
                    newAnimation.name = animationName;

                    // Add frame
                    newAnimation.AddFrame(frame);

                    // Finally push back to loaded animaitons
                    m_animations.push_back(newAnimation);
                }
            }
        }

        fileData.close();
        jsonData.dump();

        std::cout << "Animation from " << in_dirPath << " Loaded" << std::endl;
        return true;
    }

    void AnimationPlayer::SetActiveAnimation(const Animation* in_pAnim)
    {
        m_pActiveAnimation = in_pAnim;

    }

    void AnimationPlayer::SetFlipped(bool in_flipped)
    {
        MEGA_ASSERT(false, "Animation SetFlipped() doesnt work yet lol");
        /*Vec3F currentRot = m_tile.GetRotation();

        if (in_flipped) {
            m_tile.SetRotation(Vec3(currentRot.x, PI, currentRot.z));
        }
        else {
            m_tile.SetRotation(Vec3(currentRot.x, 0, currentRot.z));
        }
        */
    }
}