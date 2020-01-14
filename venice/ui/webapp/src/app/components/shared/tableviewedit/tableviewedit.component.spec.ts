import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ChangeDetectorRef, Component, Input } from '@angular/core';
import { ComponentFixture, TestBed, fakeAsync, tick, discardPeriodicTasks } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { MatIconRegistry } from '@angular/material';
import { By } from '@angular/platform-browser';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { TestingUtility } from '@app/common/TestingUtility';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MessageService } from '@app/services/message.service';
import { BehaviorSubject } from 'rxjs';
import { TablevieweditHTMLComponent, TablevieweditAbstract, CreationForm } from './tableviewedit.component';
import { LazyrenderComponent } from '../lazyrender/lazyrender.component';
import { SorticonComponent } from '../sorticon/sorticon.component';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TableCol, CustomExportMap } from '.';
import { RoleGuardDirective } from '../directives/roleGuard.directive';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { AuthService as AuthServiceGen } from '@app/services/generated/auth.service';
import { AuthService } from '@app/services/auth.service';
import { FlexLayoutModule } from '@angular/flex-layout';
import { BaseModel } from '@sdk/v1/models/generated/basemodel/base-model';
import { ApiObjectMeta, IApiObjectMeta } from '@sdk/v1/models/generated/auth';
import { FormGroup, FormsModule } from '@angular/forms';
import { AdvancedSearchComponent } from '../advanced-search/advanced-search.component';
import { WidgetsModule } from 'web-app-framework';



export class TestTablevieweditRBAC {
  fixture: ComponentFixture<any>;
  skipEdit: boolean = false;
  createOperationPermissions: UIRolePermissions[];

  constructor(protected permissonBase: string, protected extraPermissions: string[] = []) {
    this.createOperationPermissions = this.buildCreatePermissionLists();
  }

  /**
   * This API builds permission list for "object" create operation
   * For example, create a techsupport request [monitoringtechsupportrequest, clusternode_read, clusterdistributedservicecard_read] permissions
   * See:
   * techsupport.component.spec.ts -- > describe('RBAC', () has example
   */
  buildCreatePermissionLists(): UIRolePermissions[] {
    const list: UIRolePermissions[] = [UIRolePermissions[this.permissonBase + '_create']];
    for (let i = 0; this.extraPermissions && i < this.extraPermissions.length; i++) {
      list.push(UIRolePermissions[this.extraPermissions[i]]);
    }
    return list;
  }
  /**
   * Runs basic rbac test to check for create, edit, and delete buttons.
   * Meant for use with components that use table view edit.
   */
  runTests() {
    let toolbarSpy: jasmine.Spy;

    beforeEach(() => {
      TestingUtility.removeAllPermissions();
      const controllerService = TestBed.get(ControllerService);
      toolbarSpy = spyOn(controllerService, 'setToolbarData');
    });

    it('Admin user', () => {
      TestingUtility.setAllPermissions();
      this.fixture.detectChanges();

      // create button
      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(1);
      const actionButtons = this.fixture.debugElement.queryAll(By.css('.global-table-action-icon'));
      if (!this.skipEdit) {
        // settings button
        expect(actionButtons.length).toBe(3);
        expect(actionButtons[0].nativeElement.textContent).toBe('settings');
        // edit button
        expect(actionButtons[1].nativeElement.textContent).toBe('edit');
        // delete button
        expect(actionButtons[2].nativeElement.textContent).toBe('delete');
      } else {
        expect(actionButtons.length).toBe(2);
        // settings button
        expect(actionButtons[0].nativeElement.textContent).toBe('settings');
        // delete button
        expect(actionButtons[1].nativeElement.textContent).toBe('delete');
      }
    });

    it('create access only', () => {
      TestingUtility.addPermissions(this.createOperationPermissions);
      this.fixture.detectChanges();

      // create button
      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(1);

      // edit button not present
      const actionButtons = this.fixture.debugElement.queryAll(By.css('.global-table-action-icon'));
      expect(actionButtons.length).toBe(1);
    });

    it('edit access only', () => {
      if (this.skipEdit) {
        return;
      }
      TestingUtility.addPermissions([UIRolePermissions[this.permissonBase + '_update']]);
      this.fixture.detectChanges();

      // create button
      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(0);
      // edit button not present
      const actionButtons = this.fixture.debugElement.queryAll(By.css('.global-table-action-icon'));
      expect(actionButtons.length).toBe(2);
      expect(actionButtons[0].nativeElement.textContent).toBe('settings');
      expect(actionButtons[1].nativeElement.textContent).toBe('edit');
    });

    it('delete access only', () => {
      TestingUtility.addPermissions([UIRolePermissions[this.permissonBase + '_delete']]);
      this.fixture.detectChanges();

      // create button
      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(0);
      // edit button not present
      const actionButtons = this.fixture.debugElement.queryAll(By.css('.global-table-action-icon'));
      expect(actionButtons.length).toBe(2);
      expect(actionButtons[0].nativeElement.textContent).toBe('settings');
      expect(actionButtons[1].nativeElement.textContent).toBe('delete');
    });

    it('no access', () => {
      this.fixture.detectChanges();

      // create button
      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(0);
      // edit and delete button not present
      const actionButtons = this.fixture.debugElement.queryAll(By.css('.global-table-action-icon'));
      expect(actionButtons.length).toBe(1);
    });
  }
}

interface IDummyObj {
  name: string;
  'mod-time': number;
}

class DummyObj implements IDummyObj {
  name: string;
  'mod-time': number;
}

@Component({
  template: `
  <app-tableviewedit fxLayout="column" class="eventpolicy-widget"
                    [creatingMode]="creatingMode"
                    [showRowExpand]="showRowExpand"
                    [data]="dataObjects"
                    [cols]="cols"
                    [runDoCheck]=false
                    [expandedRowData]="expandedRowData"
                    dataKey="name"
                    sortField="mod-time"
                    [sortOrder]="1"
                    [captionTemplate]="captionTemplate"
                    [bodyTemplate]="bodyTemplate"
                    [actionTemplate]="actionTemplate"
                    [expandTemplate]="expandTemplate"
                    [createTemplate]="createTemplate"
                    (rowExpandAnimationComplete)="rowExpandAnimationComplete($event)"
                    >
  </app-tableviewedit>

  <ng-template #createTemplate>
  <div class="dummy-create">Create Template</div>
</ng-template>

  <!-- TABLE HEADER -->
<ng-template #captionTemplate let-count="count">
  <div class="dummy-header">Header {{count}}</div>
</ng-template>

<ng-template #bodyTemplate let-rowData let-col="col">
  <div class="dummy-body">{{rowData[col.field]}}</div>
</ng-template>

<ng-template #actionTemplate let-rowData>
  <div class="dummy-action">Action Template</div>
</ng-template>

<ng-template #expandTemplate let-rowData let-columns="columns">
  <div class="dummy-expand">Expand Template {{ rowData.name }}</div>
</ng-template>
            `
})
class DummyComponent extends TablevieweditAbstract<IDummyObj, DummyObj> {
  @Input() dataObjects: ReadonlyArray<DummyObj> = [];

  exportFilename = 'test-export';
  exportMap: CustomExportMap = {};

  cols: TableCol[] = [
    { field: 'name', header: 'Name', width: 50 },
    { field: 'mod-time', header: 'Time', width: 50 }
  ];

  isTabComponent = false;

  disableTableWhenRowExpanded = true;

  constructor(protected controllerService: ControllerService,
    protected cdr: ChangeDetectorRef,
    protected uiconfigService: UIConfigsService) {
    super(controllerService, cdr, uiconfigService);
  }

  postNgInit() { }

  getClassName() {
    return this.constructor.name;
  }

  // Will be mocked in test
  setDefaultToolbar() {
    this.controllerService.setToolbarData({});
  }

  // Will be mocked in test
  deleteRecord(item: DummyObj) {
    return new BehaviorSubject<any>(
      item
    ).asObservable();
  }

  generateDeleteConfirmMsg(object) {
    return 'confirm msg ' + object.name;
  }

  generateDeleteSuccessMsg(object) {
    return 'delete msg ' + object.name;
  }
}

interface IDummyCreateObj {
  meta: IApiObjectMeta;
}

class DummyCreateObj extends BaseModel implements IDummyCreateObj {
  'meta': ApiObjectMeta = null;

  constructor() {
    super();
    this['meta'] = new ApiObjectMeta();
  }

  getPropInfo() {
    return null;
  }

  protected getFormGroup(): FormGroup {
    if (!this._formGroup) {
      this._formGroup = new FormGroup({
        'meta': this['meta'].$formGroup,
      });
      // We force recalculation of controls under a form group
      Object.keys((this._formGroup.get('meta') as FormGroup).controls).forEach(field => {
        const control = this._formGroup.get('meta').get(field);
        control.updateValueAndValidity();
      });
    }
    return this._formGroup;
  }
}

@Component({
  template: `
    <div>Hello</div>
            `
})
class DummyCreateComponent extends CreationForm<IDummyCreateObj, DummyCreateObj> {
  constructor(protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected cdr: ChangeDetectorRef) {
    super(controllerService, uiconfigsService, DummyCreateObj);
  }

  getClassName() {
    return this.constructor.name;
  }

  // Empty Hook
  postNgInit() { }

  // Empty Hook
  isFormValid() {
    return true;
  }

  // Will be mocked in test
  setToolbar() {
    this.controllerService.setToolbarData({});
  }

  createObject(object) {
    return new BehaviorSubject<any>(
      object
    ).asObservable();
  }

  updateObject(object) {
    return new BehaviorSubject<any>(
      object
    ).asObservable();
  }

  generateCreateSuccessMsg(object) {
    return 'create msg ' + object.name;
  }

  generateUpdateSuccessMsg(object) {
    return 'update msg ' + object.name;
  }
}


describe('TablevieweditComponent', () => {
  let component: DummyComponent;
  let fixture: ComponentFixture<DummyComponent>;
  const initialData = [
    {
      name: 'obj1',
      'mod-time': 10
    },
    {
      name: 'obj2',
      'mod-time': 9
    },
    {
      name: 'obj3',
      'mod-time': 8
    },
    {
      name: 'obj4',
      'mod-time': 7
    },
    {
      name: 'obj5',
      'mod-time': 6
    },
    {
      name: 'obj6',
      'mod-time': 5
    },
  ];

  function getHeaderTemplate() {
    return fixture.debugElement.query(By.css('.dummy-header'));
  }

  function getActionTemplate() {
    return fixture.debugElement.query(By.css('.dummy-action'));
  }

  function getCreateTemplate() {
    return fixture.debugElement.query(By.css('.dummy-create'));
  }

  function getExpandTemplate() {
    return fixture.debugElement.query(By.css('.dummy-expand'));
  }

  function getOverlay() {
    return fixture.debugElement.query(By.css('.tableviewedit-overlay'));
  }

  function getTable() {
    return fixture.debugElement.query(By.css('.ui-table-scrollable-body tbody'));
  }



  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [
        DummyComponent,
        DummyCreateComponent,
        TablevieweditHTMLComponent,
        LazyrenderComponent,
        SorticonComponent,
        RoleGuardDirective,
        AdvancedSearchComponent
      ],
      imports: [
        NoopAnimationsModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        HttpClientTestingModule,
        FlexLayoutModule,
        FormsModule,
        WidgetsModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        AuthService,
        AuthServiceGen,
        LogPublishersService,
        MatIconRegistry,
        MessageService,
        ChangeDetectorRef,
        UIConfigsService
      ]
    });
  });

  it('should create tableviewedit', fakeAsync(() => {
    fixture = TestBed.createComponent(DummyComponent);
    component = fixture.componentInstance;
    component.dataObjects = initialData;
    const toolbarSpy = spyOn(component, 'setDefaultToolbar');

    fixture.detectChanges();
    expect(component).toBeTruthy();
    fixture.detectChanges();
    tick();
    fixture.detectChanges();

    component.tableContainer.lazyRenderWrapper.stopResizeTableInterval();
    clearTimeout(component.tableContainer.lazyRenderWrapper.resizeTimeout);

    // Default toolbar should have been called.
    expect(toolbarSpy).toHaveBeenCalled();
    // Should show header with item count
    expect(getHeaderTemplate()).toBeTruthy();
    expect(getHeaderTemplate().nativeElement.textContent).toBe('Header 6');
    // Items should be sorted by mod time ascending
    TestingUtility.verifyTable(initialData.reverse(), component.cols, getTable(), {}, 'Action Template');
    // Action template should be there
    expect(getActionTemplate()).toBeTruthy();
    // creation, expand, overlay should NOT be there
    expect(getCreateTemplate()).toBeNull();
    expect(getExpandTemplate()).toBeNull();
    expect(getOverlay()).toBeNull();

    // Create New obj trigger
    // SHould see creation template
    // Should have overlay
    component.createNewObject();
    fixture.detectChanges();
    tick();
    expect(getCreateTemplate()).toBeTruthy();
    expect(getExpandTemplate()).toBeNull();
    expect(getOverlay()).toBeTruthy();


    // Cancel new obj creation form
    component.creationFormClose();
    fixture.detectChanges();
    fixture.detectChanges();
    tick();
    expect(getCreateTemplate()).toBeNull();
    expect(getExpandTemplate()).toBeNull();
    expect(getOverlay()).toBeNull();


    // Force into edit mode
    // Should see overlay and row expand
    component.expandRowRequest(null, initialData[0]);
    fixture.detectChanges();
    tick();
    expect(getCreateTemplate()).toBeNull();
    expect(getExpandTemplate()).toBeTruthy();
    expect(getOverlay()).toBeTruthy();
    expect(getExpandTemplate().nativeElement.textContent).toContain('obj6');


    // Change the dataObjects content, view should be the same
    const newItems = [
      {
        name: 'obj8',
        'mod-time': 3
      },
      {
        name: 'obj7',
        'mod-time': 4
      },
    ];
    component.dataObjects = newItems;
    fixture.detectChanges();

    expect(getHeaderTemplate().nativeElement.textContent).toBe('Header 6');
    const rows = getTable().queryAll(By.css('tr'));
    // 6 items + 1 for expanded row
    expect(rows.length).toBe(7, 'Data did not match number of entries in the table');

    // exit edit mode should see new objects
    component.closeRowExpand();
    fixture.detectChanges();
    tick();
    expect(getHeaderTemplate().nativeElement.textContent).toBe('Header 2');
    TestingUtility.verifyTable(newItems, component.cols, getTable(), {}, 'Action Template');

    // delete an object and check for confirm messages

    const confirmSpy = spyOn(TestBed.get(ConfirmationService), 'confirm');
    const toasterSpy = spyOn(TestBed.get(ControllerService), 'invokeSuccessToaster');
    component.onDeleteRecord(null, newItems[0]);
    expect(confirmSpy).toHaveBeenCalled();
    const confirmArgs = confirmSpy.calls.mostRecent().args;
    TestBed.get(ConfirmationService);
    confirmArgs[0].header = component.generateDeleteConfirmMsg(newItems[0]);

    // Triggering accept
    confirmArgs[0].accept();

    expect(toasterSpy).toHaveBeenCalledWith('Delete Successful', component.generateDeleteSuccessMsg(newItems[0]));
    discardPeriodicTasks();
    tick(2000);

  }));

  it('creation form create', () => {
    const createFixture = TestBed.createComponent(DummyCreateComponent);
    const createComponent = createFixture.componentInstance;
    const postNgInitSpy = spyOn(createComponent, 'postNgInit');
    const toolbarSpy = spyOn(createComponent, 'setToolbar');
    createFixture.detectChanges();
    expect(createComponent.newObject).toBeTruthy();
    expect(postNgInitSpy).toHaveBeenCalled();
    expect(toolbarSpy).toHaveBeenCalled();

    const toasterSpy = spyOn(TestBed.get(ControllerService), 'invokeSuccessToaster');
    const createSpy = spyOn(createComponent, 'createObject').and.callThrough();
    createComponent.newObject.$formGroup.get('meta.name').setValue('policy1');
    createComponent.saveObject();
    createFixture.detectChanges();

    expect(createSpy).toHaveBeenCalled();
    expect(toasterSpy).toHaveBeenCalled();
    expect(toasterSpy).toHaveBeenCalledWith('Create Successful', createComponent.generateCreateSuccessMsg(createComponent.newObject));

  });

  it('creation form update', () => {
    const createFixture = TestBed.createComponent(DummyCreateComponent);
    const createComponent = createFixture.componentInstance;
    createComponent.isInline = true;
    createComponent.objectData = { meta: { name: 'policy1' } };
    const postNgInitSpy = spyOn(createComponent, 'postNgInit');
    const toolbarSpy = spyOn(createComponent, 'setToolbar');
    createFixture.detectChanges();
    expect(createComponent.newObject).toBeTruthy();
    expect(postNgInitSpy).toHaveBeenCalled();
    expect(toolbarSpy).toHaveBeenCalledTimes(0);

    const toasterSpy = spyOn(TestBed.get(ControllerService), 'invokeSuccessToaster');
    const createSpy = spyOn(createComponent, 'updateObject').and.callThrough();
    createComponent.newObject.$formGroup.get('meta.name').setValue('policy1');
    createComponent.saveObject();
    createFixture.detectChanges();

    expect(createSpy).toHaveBeenCalled();
    expect(toasterSpy).toHaveBeenCalled();
    expect(toasterSpy).toHaveBeenCalledWith('Update Successful', createComponent.generateUpdateSuccessMsg(createComponent.objectData));

  });
});
