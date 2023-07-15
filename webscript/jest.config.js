/** @type {import('@ts-jest/dist/types').InitialOptionsTsJest} */
module.exports = {
  preset: 'ts-jest',
  testEnvironment: 'jsdom',
  rootDir: './src',
  clearMocks: true,
  setupFiles: ['<rootDir>/setup-jest.ts'],
};
