/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined(INCLUDED_ROTATION_H)
#define INCLUDED_ROTATION_H

#include "ientity.h"

#include "math/quaternion.h"
#include "generic/callback.h"
#include "stringio.h"

#include "angle.h"

typedef float Float9[9];

inline void default_rotation(Float9 rotation)
{
  rotation[0] = 1;
  rotation[1] = 0;
  rotation[2] = 0;
  rotation[3] = 0;
  rotation[4] = 1;
  rotation[5] = 0;
  rotation[6] = 0;
  rotation[7] = 0;
  rotation[8] = 1;
}
inline void write_rotation(const Float9 rotation, Entity* entity, const char* key = "rotation")
{
  if(rotation[0] == 1
    && rotation[1] == 0
    && rotation[2] == 0
    && rotation[3] == 0
    && rotation[4] == 1
    && rotation[5] == 0
    && rotation[6] == 0
    && rotation[7] == 0
    && rotation[8] == 1)
  {
    entity->setKeyValue(key, "");
  }
  else
  {
	std::ostringstream value;
    value << rotation[0] << ' '
      << rotation[1] << ' '
      << rotation[2] << ' '
      << rotation[3] << ' '
      << rotation[4] << ' '
      << rotation[5] << ' '
      << rotation[6] << ' '
      << rotation[7] << ' '
      << rotation[8];
    entity->setKeyValue(key, value.str());
  }
}
inline void read_rotation(Float9 rotation, const std::string& value)
{
  if(!string_parse_vector(value.c_str(), rotation, rotation + 9))
  {
    default_rotation(rotation);
  }
}

inline Matrix4 rotation_toMatrix(const Float9 rotation)
{
  return Matrix4::byColumns(
    rotation[0],
    rotation[1],
    rotation[2],
    0,
    rotation[3],
    rotation[4],
    rotation[5],
    0,
    rotation[6],
    rotation[7],
    rotation[8],
    0,
    0,
    0,
    0,
    1
  );
}

inline void rotation_fromMatrix(Float9 rotation, const Matrix4& matrix)
{
  rotation[0] = static_cast<float>(matrix.xx());
  rotation[1] = static_cast<float>(matrix.xy());
  rotation[2] = static_cast<float>(matrix.xz());
  rotation[3] = static_cast<float>(matrix.yx());
  rotation[4] = static_cast<float>(matrix.yy());
  rotation[5] = static_cast<float>(matrix.yz());
  rotation[6] = static_cast<float>(matrix.zx());
  rotation[7] = static_cast<float>(matrix.zy());
  rotation[8] = static_cast<float>(matrix.zz());
}

inline void rotation_assign(Float9 rotation, const Float9 other)
{
  rotation[0] = other[0];
  rotation[1] = other[1];
  rotation[2] = other[2];
  rotation[3] = other[3];
  rotation[4] = other[4];
  rotation[5] = other[5];
  rotation[6] = other[6];
  rotation[7] = other[7];
  rotation[8] = other[8];
}

inline void rotation_rotate(Float9 rotation, const Quaternion& rotate)
{
  rotation_fromMatrix(rotation,
    matrix4_multiplied_by_matrix4(
      rotation_toMatrix(rotation),
      matrix4_rotation_for_quaternion_quantised(rotate)
    )
  );
}

inline void read_angle(Float9 rotation, const std::string& value)
{
  float angle;
  if(!string_parse_float(value.c_str(), angle))
  {
    default_rotation(rotation);
  }
  else
  {
    rotation_fromMatrix(rotation,  matrix4_rotation_for_z_degrees(angle));
  }
}

class RotationKey
{
  Callback m_rotationChanged;
public:
  Float9 m_rotation;


  RotationKey(const Callback& rotationChanged)
    : m_rotationChanged(rotationChanged)
  {
    default_rotation(m_rotation);
  }

  void angleChanged(const std::string& value)
  {
    read_angle(m_rotation, value);
    m_rotationChanged();
  }
  typedef MemberCaller1<RotationKey, const std::string&, &RotationKey::angleChanged> AngleChangedCaller;

  void rotationChanged(const std::string& value)
  {
    read_rotation(m_rotation, value);
    m_rotationChanged();
  }
  typedef MemberCaller1<RotationKey, const std::string&, &RotationKey::rotationChanged> RotationChangedCaller;

  void write(Entity* entity, bool isModel = false) const
  {
  	Vector3 euler = matrix4_get_rotation_euler_xyz_degrees(rotation_toMatrix(m_rotation));
  	// greebo: Prevent the "angle" key from being used for models, they should always
  	// have a rotation matrix written to their spawnargs. This should fix
  	// the models hopping around after transforms
    if(euler[0] == 0 && euler[1] == 0 && !isModel)
    {
      entity->setKeyValue("rotation", "");
      write_angle(static_cast<float>(euler[2]), entity);
    }
    else
    {
      entity->setKeyValue("angle", "");
      write_rotation(m_rotation, entity);
    }
  }
};

#endif
