import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { WidgetsModule } from 'web-app-framework';
import { PrimengModule } from '@app/lib/primeng.module';

import { NaplesComponent } from './naples.component';
import { DsbdwidgetheaderComponent } from '@app/components/shared/dsbdwidgetheader/dsbdwidgetheader.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

describe('NaplesComponent', () => {
  let component: NaplesComponent;
  let fixture: ComponentFixture<NaplesComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [NaplesComponent, DsbdwidgetheaderComponent],
      imports: [
        WidgetsModule,
        PrimengModule,
        MaterialdesignModule
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NaplesComponent);
    component = fixture.componentInstance;
    component.id = 'test_id';
    component.background_img_left = {
      url: 'test.com'
    };
    component.background_img_right = {
      url: 'test.com'
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
