import * as path from 'path';
import { generateMetricMetadata } from './generators/metrics';
import { generateHelpDocs, Config as HelpConfig } from './generators/helpdocs';
import { generateEventTypesFile } from './generators/events';
import { generateUIPermissionsFile } from './generators/permissions';
import { Pipeline} from './generators/metrics';
import { getManifest } from './generators/utils';
import * as handlebars from 'handlebars';
import { genModelAndServices, Config as ModelConfig } from './generators/models';

// Expects to run from root of venice-sdk

// Generate folders for generated files, and remove any files if there are any
const version = 'v1';
// const genModelFolder = 
// const genServiceFolder =

handlebars.registerHelper('toJSON', function (obj) {
  return JSON.stringify(obj, null, 2);
});
handlebars.registerHelper('ifeq', function (a, b, options) {
  if (a == b) { return options.fn(this); }
  return options.inverse(this);
});

handlebars.registerHelper('ifnoteq', function (a, b, options) {
  if (a != b) { return options.fn(this); }
  return options.inverse(this);
});

// gen models
const modelConfig: ModelConfig  = {
  genModelFolder: path.join(process.cwd(), 'v1/', 'models/generated/'),
  genServiceFolder:  path.join(process.cwd(), 'v1/', 'services/generated/'),
}

genModelAndServices(modelConfig);

// Event types
const eventPath = path.join(process.cwd(), '../../../events/generated/eventtypes/eventtypes.json');
const outputFile = version + '/models/generated/eventtypes.ts';
generateEventTypesFile(eventPath, outputFile);


// permissions
const manifest = getManifest()
generateUIPermissionsFile(manifest);


// Help docs
const helpConfig: HelpConfig = {
  helpDir: "../docs/help",
  tempDir: "./tmp",
  htmlDir: "../webapp/src/assets/generated/docs/help/",
  htmlLinkBase: "/assets/generated/docs/help"
}
generateHelpDocs(helpConfig);


// Metrics doc
const metricsOutput = path.join(process.cwd(), 'metrics/', 'generated/');
const pipelines: Pipeline[] = [
  {
    pipeline: "Iris",
    basePath: '../../../metrics/generated/',
  },
  {
    pipeline: "Apulu",
    basePath: '../../../metrics/apulu/generated',
  }
]
generateMetricMetadata(pipelines, metricsOutput);









