namespace TestHelpers {
	constexpr float DEFAULT_EPSILON = 1.0e-5;

	inline constexpr bool check_vec3_close_vec3(const glm::vec3& vec1, const glm::vec3& vec2, float eps = DEFAULT_EPSILON) {
		glm::vec3 dif = glm::abs(vec1 - vec2);
		return dif.x <= eps &&
			dif.y <= eps &&
			dif.z <= eps;
	}

	inline constexpr bool check_vec3_close_float(const glm::vec3& vec1, float flt, float eps = DEFAULT_EPSILON) {
		glm::vec3 dif = glm::abs(vec1 - flt);
		return dif.x <= eps &&
			dif.y <= eps &&
			dif.z <= eps;
	}
};