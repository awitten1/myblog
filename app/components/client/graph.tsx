'use client'

import React, { useRef, useState, createContext, useCallback, useMemo } from "react"

interface Point {
  x: number
  y: number
  input_size: number
  cycles: number
}

interface LineMeta {
  id: string
  label: string
  color: string
}

interface GraphContextType {
  registerPoints: (id: string, points: Point[]) => void
  unregisterPoints: (id: string) => void
  registerLine: (meta: LineMeta) => void
  unregisterLine: (id: string) => void
}

export const GraphContext = createContext<GraphContextType | null>(null)

function range(start: number, end?: number, step = 1) {
  if (end === undefined) {
    end = start
    start = 0
  }
  let ret: number[] = []
  for (let i = start; i <= (end as number); i += step) {
    ret.push(i)
  }
  return ret;
}

interface TestSvgProps {
  children: React.ReactNode
  height: number
  width: number
  marginLeft?: number
  marginBottom?: number
}

export function Grid({ children, height, width, marginLeft = 0, marginBottom = 0 }: TestSvgProps) {
  const horizontalLineRef = useRef<SVGLineElement>(null);
  const verticalLineRef = useRef<SVGLineElement>(null);
  const [activePoint, setActivePoint] = useState<Point | null>(null);
  const pointsMapRef = useRef<Record<string, Point[]>>({});
  const [lines, setLines] = useState<Record<string, LineMeta>>({});

  const registerPoints = useCallback((id: string, points: Point[]) => {
    pointsMapRef.current[id] = points;
  }, []);

  const unregisterPoints = useCallback((id: string) => {
    delete pointsMapRef.current[id];
  }, []);

  const registerLine = useCallback((meta: LineMeta) => {
    setLines(prev => ({ ...prev, [meta.id]: meta }));
  }, []);

  const unregisterLine = useCallback((id: string) => {
    setLines(prev => {
      const next = { ...prev };
      delete next[id];
      return next;
    });
  }, []);

  const sep = 30

  const handleMouseMove = (event: React.MouseEvent<SVGSVGElement>) => {
    const rect = event.currentTarget.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;

    let nearest: Point | null = null;
    let minDist = Infinity;

    for (const points of Object.values(pointsMapRef.current)) {
      for (const p of points) {
        const dist = Math.sqrt(Math.pow(x - p.x, 2) + Math.pow(y - p.y, 2));
        if (dist < minDist) {
          minDist = dist;
          nearest = p;
        }
      }
    }

    if (nearest && minDist < 50) {
      if (activePoint?.x !== nearest.x || activePoint?.y !== nearest.y) {
        setActivePoint(nearest);
      }
      if (horizontalLineRef.current) {
        horizontalLineRef.current.setAttribute('y1', String(nearest.y));
        horizontalLineRef.current.setAttribute('y2', String(nearest.y));
      }
      if (verticalLineRef.current) {
        verticalLineRef.current.setAttribute('x1', String(nearest.x));
        verticalLineRef.current.setAttribute('x2', String(nearest.x));
      }
    } else {
      if (activePoint !== null) {
        setActivePoint(null);
      }
      if (horizontalLineRef.current) {
        horizontalLineRef.current.setAttribute('y1', String(y));
        horizontalLineRef.current.setAttribute('y2', String(y));
      }
      if (verticalLineRef.current) {
        verticalLineRef.current.setAttribute('x1', String(x));
        verticalLineRef.current.setAttribute('x2', String(x));
      }
    }
  };

  const contextValue = useMemo(() => ({
    registerPoints,
    unregisterPoints,
    registerLine,
    unregisterLine
  }), [registerPoints, unregisterPoints, registerLine, unregisterLine]);

  return (
    <GraphContext value={contextValue}>
      <div style={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        width: '100%',
        marginTop: '60px',
        marginBottom: marginBottom ? `${marginBottom + 20}px` : '40px'
      }}>
        <div style={{
          position: 'relative',
          width: `${width}px`,
        }}>
          {/* Legend */}
          <div style={{
            position: 'absolute',
            top: -30,
            right: 0,
            display: 'flex',
            gap: '16px',
            fontSize: '12px',
            color: '#888'
          }}>
            {Object.values(lines).map(line => (
              <div key={line.id} style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
                <div style={{ width: '12px', height: '2px', background: line.color }} />
                <span>{line.label}</span>
              </div>
            ))}
          </div>

          <svg
            width={width}
            height={height}
            onMouseMove={handleMouseMove}
            onMouseLeave={() => setActivePoint(null)}
            style={{ cursor: 'crosshair', background: '#000', overflow: 'visible' }}
          >
            <g fill="white">
              <rect width={width} height={height}></rect>
            </g>

            <g fill="white" stroke="#070707ff" strokeWidth="0.5">
              {range(0, width, sep).map((i) => (
                <line key={`v-${i}`} x1={i} y1={0} x2={i} y2={height}></line>
              ))}

              {range(0, height, sep).map((i) => (
                <line key={`h-${i}`} x1={0} y1={i} x2={width} y2={i}></line>
              ))}
            </g>

            <g stroke="rgba(255, 255, 255, 0.2)" strokeWidth="1" strokeDasharray="4 4">
              <line ref={verticalLineRef} x1={0} y1={0} x2={0} y2={height} />
              <line ref={horizontalLineRef} x1={0} y1={0} x2={width} y2={0} />
            </g>

            {children}

            {/* Axes Labels */}
            {marginBottom > 0 && (
              <text
                x={width / 2}
                y={height + marginBottom - 10}
                fill="#888"
                textAnchor="middle"
                fontSize="12"
                style={{ textTransform: 'uppercase', letterSpacing: '0.1em' }}
              >
                Input Size
              </text>
            )}

            {marginLeft > 0 && (
              <text
                transform={`translate(${-marginLeft + 20}, ${height / 2}) rotate(-90)`}
                fill="#888"
                textAnchor="middle"
                fontSize="12"
                style={{ textTransform: 'uppercase', letterSpacing: '0.1em' }}
              >
                Cycles
              </text>
            )}

            {activePoint && (
              <circle
                cx={activePoint.x}
                cy={activePoint.y}
                r={6}
                fill="none"
                stroke="#fff"
                strokeWidth="2"
              />
            )}
          </svg>

          {activePoint && (
            <div style={{
              position: 'absolute',
              top: activePoint.y - 80,
              left: activePoint.x + 20,
              backgroundColor: 'rgba(0, 0, 0, 0.85)',
              backdropFilter: 'blur(4px)',
              color: 'white',
              padding: '10px 14px',
              borderRadius: '8px',
              fontSize: '12px',
              border: '1px solid rgba(255, 255, 255, 0.2)',
              boxShadow: '0 4px 12px rgba(0,0,0,0.5)',
              pointerEvents: 'none',
              zIndex: 10,
              display: 'flex',
              flexDirection: 'column',
              gap: '4px'
            }}>
              <div style={{ opacity: 0.7, textTransform: 'uppercase', letterSpacing: '0.05em', fontSize: '10px' }}>Performance Data</div>
              <div style={{ display: 'flex', justifyContent: 'space-between', gap: '20px' }}>
                <span>Cycles:</span>
                <span style={{ fontFamily: 'monospace', fontWeight: 'bold', color: '#10b981' }}>{activePoint.cycles.toLocaleString()}</span>
              </div>
              <div style={{ display: 'flex', justifyContent: 'space-between', gap: '20px' }}>
                <span>Input Size:</span>
                <span style={{ fontFamily: 'monospace', fontWeight: 'bold' }}>{activePoint.input_size}</span>
              </div>
            </div>
          )}
        </div>
      </div>
    </GraphContext>
  )
}