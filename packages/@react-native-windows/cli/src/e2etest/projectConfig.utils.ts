import fs from '@react-native-windows/fs'
import path from 'path';
import { copyAndReplace } from '../generator-common';

export const templateRoot = path.resolve('../../../vnext/template');

export const testProjectGuid = '{416476D5-974A-4EE2-8145-4E331297247E}';

export async function tryMkdir(dir: string): Promise<void> {
  try {
    await fs.mkdir(dir, {recursive: true});
  } catch (err) {}
}

export async function ensureWinUI3Project(folder: string) {
  const windowsDir = path.join(folder, 'windows');
  if (fs.existsSync(windowsDir)) {
    await fs.rmdir(windowsDir, {recursive: true});
  }
  await tryMkdir(windowsDir);

  const replacements = {
    name: 'WithWinUI3',
    namespace: 'WithWinUI3',
    useMustache: true,
    projectGuidUpper: testProjectGuid,
    projectGuidLower: testProjectGuid.toLowerCase(),
    useWinUI3: false,
    useHermes: false,
    useExperimentalNuget: false,
  };

  await copyAndReplace(
    path.join(templateRoot, 'cpp-app/proj/MyApp.sln'),
    path.join(windowsDir, 'WithWinUI3.sln'),
    replacements,
    null
  );

  const projDir = path.join(windowsDir, 'WithWinUI3');
  await tryMkdir(projDir);

  await copyAndReplace(
    path.join(templateRoot, 'cpp-app/proj/MyApp.vcxproj'),
    path.join(projDir, 'WithWinUI3.vcxproj'),
    replacements,
    null
  );

  await copyAndReplace(
    path.join(templateRoot, 'shared-app/proj/ExperimentalFeatures.props'),
    path.join(windowsDir, 'ExperimentalFeatures.props'),
    replacements,
    null
  );
}
