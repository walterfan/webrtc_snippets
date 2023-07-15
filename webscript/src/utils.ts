export function generateSsrc(): number {
    return Math.floor(Math.random() * 0xffffffff) + 1;
}