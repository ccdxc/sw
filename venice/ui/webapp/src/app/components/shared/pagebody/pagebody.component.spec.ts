import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { PagebodyComponent } from './pagebody.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

describe('PagebodyComponent', () => {
  let component: PagebodyComponent;
  let fixture: ComponentFixture<PagebodyComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [PagebodyComponent,

      ],
      imports: [
        MaterialdesignModule
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(PagebodyComponent);
    component = fixture.componentInstance;
    component.icon = {
      margin: {
        top: '5px',
        left: '5px',
        url: ''
      }
    };
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
