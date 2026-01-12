import Link from 'next/link'

import { ThemeToggle } from '../client/ThemeToggle'

const navItems = {
  '/': {
    name: 'home',
  },
  '/blog': {
    name: 'blog',
  },
}

export function Navbar() {
  return (
    <nav id="nav">
      {Object.entries(navItems).map(([path, { name }]) => {
        return (
          <Link key={path} href={path}>
            {name}
          </Link>
        )
      })}
      <div style={{ marginLeft: 'auto' }}>
        <ThemeToggle />
      </div>
    </nav>
  )
}
