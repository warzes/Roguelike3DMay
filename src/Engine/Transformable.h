#pragma once

// The coordinate space in which to operate.
enum class Space
{
	World, // Transform in world coordinates
	Self   // Transform in local (object) coordinates
};


class Transformable
{
public:
	//--------------------------------------------------------------------------
	//! \brief Empty constructor. Init position and origin to 0.
	//! Init scale to 1. Init transformation to identity matrix.
	//--------------------------------------------------------------------------
	Transformable()
		: m_origin(0),
		m_position(0),
		m_orientation(1, 0, 0, 0), // identity quaternion
		m_scale(1),
		m_local_scaling(1),
		m_transform(1.0f),
		m_inverse_transform(1.0f),
		m_transform_needs_update(false),
		m_inverse_trans_needs_update(false)
	{
	}

	//--------------------------------------------------------------------------
	//! \brief Restore states to default.
	//--------------------------------------------------------------------------
	void reset()
	{
		m_origin = glm::vec3(0);
		m_position = glm::vec3(0);
		m_orientation = glm::quat(1, 0, 0, 0);
		m_scale = glm::vec3(1);
		m_local_scaling = glm::vec3(1);
		m_transform = glm::mat4(1.0f);
		m_inverse_transform = glm::mat4(1.0f);
		m_transform_needs_update = false;
		m_inverse_trans_needs_update = false;
	}

	//--------------------------------------------------------------------------
	//! \brief Return the right vector (local X axis).
	//--------------------------------------------------------------------------
	glm::vec3 right() const
	{
		return glm::rotate(m_orientation, glm::vec3(1, 0, 0));
	}

	//--------------------------------------------------------------------------
	//! \brief Return the up vector (local Y axis).
	//--------------------------------------------------------------------------
	glm::vec3 up() const
	{
		return glm::rotate(m_orientation, glm::vec3(0, 1, 0));
	}

	//--------------------------------------------------------------------------
	//! \brief Return the forward vector (local Z axis, forward in left-handed).
	//--------------------------------------------------------------------------
	glm::vec3 forward() const
	{
		return glm::rotate(m_orientation, glm::vec3(0, 0, 1)); // +Z forward in LH
	}

	//--------------------------------------------------------------------------
	//! \brief Return the direction the object is facing.
	//--------------------------------------------------------------------------
	glm::vec3 direction() const
	{
		return forward(); // Already points forward in LH
	}

	//--------------------------------------------------------------------------
	//! \brief Set the origin of the object relative to the world origin.
	//--------------------------------------------------------------------------
	Transformable& origin(const glm::vec3& origin)
	{
		m_origin = origin;
		m_transform_needs_update = true;
		return *this;
	}

	//--------------------------------------------------------------------------
	//! \brief Get the origin.
	//--------------------------------------------------------------------------
	const glm::vec3& origin() const { return m_origin; }

	//--------------------------------------------------------------------------
	//! \brief Set the position in world coordinates.
	//--------------------------------------------------------------------------
	Transformable& position(const glm::vec3& pos)
	{
		m_position = pos;
		m_transform_needs_update = true;
		return *this;
	}

	//--------------------------------------------------------------------------
	//! \brief Get the world position.
	//--------------------------------------------------------------------------
	const glm::vec3& position() const { return m_position; }

	//--------------------------------------------------------------------------
	//! \brief Set local position (relative to origin).
	//--------------------------------------------------------------------------
	Transformable& local_position(const glm::vec3& pos)
	{
		m_position = pos + m_origin;
		m_transform_needs_update = true;
		return *this;
	}

	//--------------------------------------------------------------------------
	//! \brief Get local position.
	//--------------------------------------------------------------------------
	glm::vec3 local_position() const { return m_position - m_origin; }

	//--------------------------------------------------------------------------
	//! \brief Translate the object.
	//--------------------------------------------------------------------------
	Transformable& translate(const glm::vec3& offset, Space relativeTo = Space::World)
	{
		if (relativeTo == Space::World)
		{
			m_position += offset;
		}
		else
		{
			m_position += glm::rotate(m_orientation, offset);
		}
		m_transform_needs_update = true;
		return *this;
	}

	//--------------------------------------------------------------------------
	Transformable& moveRight(float offset)
	{
		m_position += offset * right();
		m_transform_needs_update = true;
		return *this;
	}

	//--------------------------------------------------------------------------
	Transformable& moveUp(float offset)
	{
		m_position += offset * up();
		m_transform_needs_update = true;
		return *this;
	}

	//--------------------------------------------------------------------------
	Transformable& moveForward(float offset)
	{
		m_position += offset * forward();
		m_transform_needs_update = true;
		return *this;
	}

	//--------------------------------------------------------------------------
	//! \brief Set absolute scale.
	//--------------------------------------------------------------------------
	Transformable& scaling(const glm::vec3& scale)
	{
		m_scale = scale;
		m_transform_needs_update = true;
		return *this;
	}

	//--------------------------------------------------------------------------
	const glm::vec3& scaling() const { return m_scale; }

	//--------------------------------------------------------------------------
	//! \brief Scale relatively.
	//--------------------------------------------------------------------------
	Transformable& scale(const glm::vec3& factor)
	{
		m_scale *= factor;
		m_transform_needs_update = true;
		return *this;
	}

	//--------------------------------------------------------------------------
	//! \brief Set local scale (does not affect children).
	//--------------------------------------------------------------------------
	Transformable& localScale(const glm::vec3& scale)
	{
		m_local_scaling = scale;
		return *this;
	}

	//--------------------------------------------------------------------------
	const glm::vec3& localScale() const { return m_local_scaling; }

	//--------------------------------------------------------------------------
	//! \brief Set orientation from quaternion.
	//--------------------------------------------------------------------------
	void attitude(const glm::quat& q)
	{
		m_orientation = glm::normalize(q);
		m_transform_needs_update = true;
	}

	//--------------------------------------------------------------------------
	const glm::quat& attitude() const { return m_orientation; }

	//--------------------------------------------------------------------------
	//! \brief Rotate by angle around axis.
	//--------------------------------------------------------------------------
	Transformable& rotate(float angle, const glm::vec3& axis, Space relativeTo = Space::Self)
	{
		glm::quat rotation = glm::angleAxis(angle, glm::normalize(axis));
		if (relativeTo == Space::Self)
		{
			m_orientation = m_orientation * rotation;
		}
		else
		{
			m_orientation = rotation * m_orientation;
		}
		m_orientation = glm::normalize(m_orientation);
		m_transform_needs_update = true;
		return *this;
	}

	//--------------------------------------------------------------------------
	Transformable& rotate(const glm::quat& q, Space relativeTo = Space::Self)
	{
		glm::quat q_norm = glm::normalize(q);
		if (relativeTo == Space::Self)
		{
			m_orientation = m_orientation * q_norm;
		}
		else
		{
			m_orientation = q_norm * m_orientation;
		}
		m_orientation = glm::normalize(m_orientation);
		m_transform_needs_update = true;
		return *this;
	}

	//--------------------------------------------------------------------------
	Transformable& pitch(float angle, Space relativeTo = Space::Self)
	{
		return rotate(angle, right(), relativeTo);
	}

	//--------------------------------------------------------------------------
	Transformable& yaw(float angle, Space relativeTo = Space::Self)
	{
		return rotate(angle, up(), relativeTo);
	}

	//--------------------------------------------------------------------------
	Transformable& roll(float angle, Space relativeTo = Space::Self)
	{
		return rotate(angle, forward(), relativeTo);
	}

	//--------------------------------------------------------------------------
	//! \brief Make object look at target using left-handed coordinates.
	//--------------------------------------------------------------------------
	void lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
	{
		m_position = position;
		glm::mat4 view = glm::lookAtLH(m_position, target, up);
		// Extract rotation: inverse of view matrix gives orientation
		glm::mat4 rot = glm::inverse(view);
		rot = glm::mat4(glm::mat3(rot)); // Remove translation
		m_orientation = glm::quat_cast(rot);
		m_transform_needs_update = true;
	}

	//--------------------------------------------------------------------------
	void lookAt(const glm::vec3& position, const glm::vec3& target)
	{
		glm::vec3 up = (std::abs(glm::dot(target - position, glm::vec3(0, 1, 0))) < 0.99f)
			? glm::vec3(0, 1, 0) : glm::vec3(0, 0, 1);
		lookAt(position, target, up);
	}

	//--------------------------------------------------------------------------
	void lookAt(const glm::vec3& target)
	{
		lookAt(m_position, target);
	}

	//--------------------------------------------------------------------------
	//! \brief Get transformation matrix: T * R * S (origin-adjusted).
	//! Order: Translate(pos - origin) * Rotate * Scale
	//--------------------------------------------------------------------------
	const glm::mat4& matrix()
	{
		if (m_transform_needs_update)
		{
			m_transform = glm::mat4(1.0f);
			// Translate to position (relative to origin)
			m_transform = glm::translate(m_transform, m_position - m_origin);
			// Apply rotation
			m_transform = m_transform * glm::toMat4(m_orientation);
			// Apply scale
			m_transform = glm::scale(m_transform, m_scale);
			m_transform_needs_update = false;
			m_inverse_trans_needs_update = true;
		}
		return m_transform;
	}

	//--------------------------------------------------------------------------
	//! \brief Get inverse transformation matrix.
	//--------------------------------------------------------------------------
	const glm::mat4& invMatrix()
	{
		if (m_inverse_trans_needs_update)
		{
			m_inverse_transform = glm::inverse(matrix());
			m_inverse_trans_needs_update = false;
		}
		return m_inverse_transform;
	}

	//--------------------------------------------------------------------------
	bool modified() const { return m_transform_needs_update; }

protected:
	glm::vec3 m_origin;
	glm::vec3 m_position;
	glm::quat m_orientation;
	glm::vec3 m_scale;
	glm::vec3 m_local_scaling;
	glm::mat4 m_transform;
	glm::mat4 m_inverse_transform;
	bool m_transform_needs_update;
	bool m_inverse_trans_needs_update;
};