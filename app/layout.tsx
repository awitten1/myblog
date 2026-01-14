import './global.css'
import 'katex/dist/katex.min.css'
import type { Metadata } from 'next'
import { GeistSans } from 'geist/font/sans'
import { GeistMono } from 'geist/font/mono'
import { Navbar } from './components/builtin/nav'
// import { Analytics } from '@vercel/analytics/react'
// import { SpeedInsights } from '@vercel/speed-insights/next'
// import Footer from './components/footer'
import { baseUrl } from './sitemap'

export const metadata: Metadata = {
  metadataBase: new URL(baseUrl),
  title: {
    default: 'Andrew Witten\'s Blog',
    template: '%s | Next.js Portfolio Starter',
  },
  description: 'This is my portfolio.',
  openGraph: {
    title: 'My Portfolio',
    description: 'This is my portfolio.',
    url: baseUrl,
    siteName: 'My Portfolio',
    locale: 'en_US',
    type: 'website',
  },
  robots: {
    index: true,
    follow: true,
    googleBot: {
      index: true,
      follow: true,
      'max-video-preview': -1,
      'max-image-preview': 'large',
      'max-snippet': -1,
    },
  },
}

const cx = (...classes) => classes.filter(Boolean).join(' ')

export default function RootLayout({
  children,
}: {
  children: React.ReactNode
}) {
  return (
    <html
      lang="en"
      suppressHydrationWarning
      className={cx(
        GeistSans.variable,
        GeistMono.variable
      )}
    >
      <head>
        <link
          rel="alternate"
          type="application/rss+xml"
          title="Andrew Witten's Blog"
          href={`${baseUrl}/rss`}
        />
      </head>
      <body className={cx(GeistSans.variable, GeistMono.variable)}>
        <script
          dangerouslySetInnerHTML={{
            __html: `(function() {
              try {
                var theme = localStorage.getItem('theme');
                var supportDarkMode = window.matchMedia('(prefers-color-scheme: dark)').matches === true;
                if (!theme && supportDarkMode) theme = 'dark';
                if (!theme) theme = 'light';
                document.documentElement.setAttribute('data-theme', theme);
              } catch (e) {}
            })()`,
          }}
        />
        <main>
          <Navbar />
          {children}
        </main>
      </body>
    </html>
  )
}
