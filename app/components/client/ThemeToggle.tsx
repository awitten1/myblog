'use client'

import React, { useEffect, useState } from 'react'

export function ThemeToggle() {
  const [theme, setTheme] = useState<'light' | 'dark'>('light')

  useEffect(() => {
    const savedTheme = localStorage.getItem('theme') as 'light' | 'dark' | null
    const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches

    if (savedTheme) {
      setTheme(savedTheme)
      document.documentElement.setAttribute('data-theme', savedTheme)
    } else if (prefersDark) {
      setTheme('dark')
      document.documentElement.setAttribute('data-theme', 'dark')
    }
  }, [])

  const toggleTheme = () => {
    const newTheme = theme === 'light' ? 'dark' : 'light'
    setTheme(newTheme)
    document.documentElement.setAttribute('data-theme', newTheme)
    localStorage.setItem('theme', newTheme)
  }

  return (
    <button
      onClick={toggleTheme}
      aria-label="Toggle theme"
      style={{
        background: 'none',
        border: 'none',
        cursor: 'pointer',
        padding: '0.5rem',
        fontSize: '1.2rem',
        color: 'var(--fg)',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        transition: 'opacity 0.2s',
      }}
      onMouseOver={(e) => (e.currentTarget.style.opacity = '0.7')}
      onMouseOut={(e) => (e.currentTarget.style.opacity = '1')}
    >
      {theme === 'light' ? '🌙' : '☀️'}
    </button>
  )
}
