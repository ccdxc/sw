import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';

export interface WorkloadNameInterface {
  fullname: string;
  name: string;
}

export class WorkloadUtility {
  /**
   * This API takes a list of workloads and print workloads
   * with both workload name and display name if it has one
   */
  public static getWorkloadNames(workloads: WorkloadWorkload[], displayNameFirst = true): WorkloadNameInterface[] {
    if (!workloads || workloads.length === 0) {
      return null;
    }
    const names = [];
    for (let i = 0; i < workloads.length; i++) {
      let fullname = workloads[i].meta.name;
      const originalName = fullname;
      let name = fullname;
      if (workloads[i].meta.labels &&
            workloads[i].meta.labels['io.pensando.vcenter.display-name']) {
        name = workloads[i].meta.labels['io.pensando.vcenter.display-name'];
        if (displayNameFirst) {
          fullname = workloads[i].meta.labels['io.pensando.vcenter.display-name'] + '(' + fullname + ')';
        } else {
          fullname += '(' + workloads[i].meta.labels['io.pensando.vcenter.display-name'] + ')';
        }
      }
      names.push({ name, fullname, originalName });
    }
    return names;
  }
}
