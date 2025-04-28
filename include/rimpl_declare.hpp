#ifndef RIMPL_DECLARE_H
#define RIMPL_DECLARE_H

#include <array>
#include <cstddef>

namespace Rimpl {

   template <typename HiddenType_t, size_t buffer_size>
   class Rimpl {
   public:
      template <typename... Args>
      Rimpl(Args&&... args);

      Rimpl(const Rimpl<HiddenType_t, buffer_size>&);
      Rimpl(Rimpl<HiddenType_t, buffer_size>&&) noexcept;

      Rimpl<HiddenType_t, buffer_size>& operator=(const Rimpl<HiddenType_t, buffer_size>& rhs);
      Rimpl<HiddenType_t, buffer_size>& operator=(Rimpl<HiddenType_t, buffer_size>&&) noexcept;

      ~Rimpl();


      // Overload operators to simplify access
      HiddenType_t* operator->()
      {
         return &get();
      }

      const HiddenType_t* operator->() const
      {
         return &get();
      }

      // Overload * operator for direct access
      HiddenType_t& operator*()
      {
         return &get();
      }

      const HiddenType_t& operator*() const
      {
         return get();
      }


      HiddenType_t& get()
      {
         return *reinterpret_cast<HiddenType_t*>(m_buffer.data());
      }

      const HiddenType_t& get() const
      {
         return *reinterpret_cast<const HiddenType_t*>(m_buffer.data());
      }

   private:

      static constexpr size_t alignment = alignof(std::max_align_t);

      // alignas to ensure the buffer is aligned to allow for the underlying type
      using Buffer_t = std::array<std::byte, buffer_size>;

      alignas(alignment) Buffer_t m_buffer {};
   };

} // namespace Rimpl

#endif // !RIMPL_DECLARE_H