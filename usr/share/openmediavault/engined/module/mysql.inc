<?php

/**
 * Copyright (C) 2010-2012 Ian Moore <imooreyahoo@gmail.com>
 * Copyright (C) 2013-2018 OpenMediaVault Plugin Developers.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

namespace OMV\Engined\Module;

use OMV\Config\Database;
use OMV\Engine\Module\IServiceStatus;
use OMV\Engine\Module\Manager as ModuleManager;
use OMV\Engine\Module\ServiceAbstract;
use OMV\Engine\Notify\Dispatcher;
use OMV\Engine\Notify\IListener;
use OMV\System\Process;
use OMV\System\SystemCtl;

class MySql extends ServiceAbstract implements
    IListener,
    IServiceStatus
{
    /** @var Database */
    private $database;

    /** @var string */
    private $dataModelPath = 'conf.service.mysql';

    /** @var string */
    private $eventMessagePath = 'org.openmediavault.conf.service.mysql';

    /** @var ModuleManager */
    private $moduleManager;

    /** @var string */
    private $serviceName = 'mysql.service';

    public function __construct(Database $database = null)
    {
        $this->database = $database ?: Database::getInstance();
    }

    /**
     * Get the module name.
     *
     * @return string
     */
    public function getName()
    {
        return 'mysql';
    }

    /**
     * Get the status of the managed service.
     *
     * @return array
     */
    public function getStatus()
    {
        $object = $this->database->get($this->dataModelPath);
        $systemCtl = new SystemCtl($this->serviceName);

        return [
            'name' => $this->getName(),
            'title' => gettext('MySQL'),
            'enabled' => $object->get('enable'),
            'running' => $systemCtl->isActive(),
        ];
    }

    /**
     * Generate the configuration.
     *
     * @return void
     */
    public function applyConfig()
    {
        $cmd = new Process('omv-mkconf', $this->getName());
        $cmd->setRedirect2to1();
        $cmd->execute();
    }

    /**
     * Start the managed service.
     *
     * @return void
     */
    public function startService()
    {
        $object = $this->database->get($this->dataModelPath);

        if ($object->get('enable')) {
            $systemCtl = new SystemCtl($this->serviceName);
            $systemCtl->enable(true);
        }
    }

    /**
     * Stop the managed service.
     *
     * @return void
     */
    public function stopService()
    {
        $systemCtl = new SystemCtl($this->serviceName);
        $systemCtl->disable(true);
    }

    /**
     * Bind listeners.
     *
     * @param Dispatcher $dispatcher The dispatcher object.
     *
     * @return void
     */
    public function bindListeners(Dispatcher $dispatcher)
    {
        $moduleManager = ModuleManager::getInstance();

        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->eventMessagePath,
            [$this, 'setDirty']
        );

        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->eventMessagePath,
            [$moduleManager->getModule('phpfpm'), 'setDirty']
        );

        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->eventMessagePath,
            [$moduleManager->getModule('webserver'), 'setDirty']
        );
    }
}
